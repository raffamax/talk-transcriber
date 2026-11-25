#include "AudioSender.h"
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>

AudioSender::AudioSender() : juce::Thread("Talk Capture sender") {}
AudioSender::~AudioSender() { signalThreadShouldExit(); stopThread(500); }
void AudioSender::start() { if (!isThreadRunning()) startThread(); }
void AudioSender::setChannelName(juce::String name) { const juce::ScopedLock lock(metadataLock_); channelName_ = std::move(name); }
void AudioSender::push(const float* samples, int count, double rate) noexcept {
    if (count <= 0) return;
    int start = 0;
    while (start < count) {
        const int amount = juce::jmin(count - start, static_cast<int>(talk_transcriber::protocol::kMaxSamplesPerPacket));
        const auto region = fifo_.write(1);
        if (region.blockSize1 > 0) { std::memcpy(blocks_[static_cast<size_t>(region.startIndex1)].data(), samples + start, static_cast<size_t>(amount) * sizeof(float)); sizes_[static_cast<size_t>(region.startIndex1)] = amount; rates_[static_cast<size_t>(region.startIndex1)] = rate; fifo_.finishedWrite(1); } else packetsDropped_++;
        start += amount;
    }
}
void AudioSender::run() {
    const int socketHandle = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socketHandle < 0) { lastSocketError_ = errno; return; }
    socketReady_ = true;

    sockaddr_in destination{};
    destination.sin_family = AF_INET;
    destination.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    destination.sin_port = htons(talk_transcriber::protocol::kDefaultPort);

    while (!threadShouldExit()) {
        if (fifo_.getNumReady() == 0) { wait(5); continue; }

        const auto region = fifo_.read(1);
        const auto index = static_cast<size_t>(region.startIndex1);
        talk_transcriber::protocol::AudioPacketHeader header{};
        header.sequence = sequence_++;
        header.sampleRate = static_cast<std::uint32_t>(rates_[index]);
        header.channels = 1;
        header.sampleCount = static_cast<std::uint16_t>(sizes_[index]);
        std::memcpy(header.instanceId, instanceId_.getRawData(), sizeof(header.instanceId));
        {
            const juce::ScopedLock lock(metadataLock_);
            channelName_.copyToUTF8(header.channelName, sizeof(header.channelName));
        }

        std::array<std::byte, sizeof(header) + talk_transcriber::protocol::kMaxSamplesPerPacket * sizeof(float)> packet{};
        std::memcpy(packet.data(), &header, sizeof(header));
        std::memcpy(packet.data() + sizeof(header), blocks_[index].data(), static_cast<size_t>(header.sampleCount) * sizeof(float));
        const auto packetBytes = sizeof(header) + static_cast<size_t>(header.sampleCount) * sizeof(float);
        packetsAttempted_++;
        if (::sendto(socketHandle, packet.data(), packetBytes, 0, reinterpret_cast<sockaddr*>(&destination), sizeof(destination)) >= 0) packetsSent_++;
        else lastSocketError_ = errno;
        fifo_.finishedRead(1);
    }
    ::close(socketHandle);
}


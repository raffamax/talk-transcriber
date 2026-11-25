#include "AudioReceiver.h"
#include <array>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

AudioReceiver::AudioReceiver(std::uint16_t port, PacketHandler handler, std::function<void()> onIdle)
    : port_(port), handler_(std::move(handler)), onIdle_(std::move(onIdle)) {}
AudioReceiver::~AudioReceiver() { stop(); }

bool AudioReceiver::start() {
    if (running_.exchange(true)) return true;
    socket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) { running_ = false; return false; }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port_);
    if (::bind(socket_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        ::close(socket_); socket_ = -1; running_ = false; return false;
    }
    worker_ = std::thread(&AudioReceiver::run, this);
    return true;
}

void AudioReceiver::stop() {
    if (!running_.exchange(false)) return;
    if (socket_ >= 0) ::shutdown(socket_, SHUT_RDWR);
    if (worker_.joinable()) worker_.join();
    if (socket_ >= 0) { ::close(socket_); socket_ = -1; }
}

void AudioReceiver::run() {
    using namespace talk_transcriber::protocol;
    std::array<std::byte, sizeof(AudioPacketHeader) + kMaxSamplesPerPacket * sizeof(float)> packet{};
    pollfd descriptor{socket_, POLLIN, 0};
    while (running_) {
        const auto ready = ::poll(&descriptor, 1, 1000);
        if (!running_) break;
        if (ready == 0) {
            if (onIdle_) onIdle_();
            continue;
        }
        if (ready < 0 && errno == EINTR) continue;
        if (ready < 0 || !(descriptor.revents & POLLIN)) break;

        const auto count = ::recv(socket_, packet.data(), packet.size(), 0);
        AudioPacketHeader header{};
        if (count < 0 || !readAudioPacketHeader(packet.data(), static_cast<std::size_t>(count), header)) continue;
        packetsReceived_++;
        if (handler_) {
            std::vector<float> samples(header.sampleCount);
            if (!samples.empty()) std::memcpy(samples.data(), packet.data() + header.headerBytes, samples.size() * sizeof(float));
            handler_(header, std::move(samples));
        }
    }
}


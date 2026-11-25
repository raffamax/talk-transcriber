#include "AudioReceiver.h"
#include <array>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

AudioReceiver::AudioReceiver(std::uint16_t port, PacketHandler handler) : port_(port), handler_(std::move(handler)) {}
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
    if (socket_ >= 0) { ::shutdown(socket_, SHUT_RDWR); ::close(socket_); socket_ = -1; }
    if (worker_.joinable()) worker_.join();
}

void AudioReceiver::run() {
    std::array<std::byte, sizeof(talk_transcriber::protocol::AudioPacketHeader) + 4096> packet{};
    while (running_) {
        const auto count = ::recv(socket_, packet.data(), packet.size(), 0);
        if (count < static_cast<ssize_t>(sizeof(talk_transcriber::protocol::AudioPacketHeader))) continue;
        talk_transcriber::protocol::AudioPacketHeader header{};
        std::memcpy(&header, packet.data(), sizeof(header));
        if (header.magic != talk_transcriber::protocol::kMagic || header.version != talk_transcriber::protocol::kVersion) continue;
        if (header.sampleCount > talk_transcriber::protocol::kMaxSamplesPerPacket) continue;
        const auto payloadBytes = static_cast<std::size_t>(header.sampleCount) * sizeof(float);
        if (static_cast<std::size_t>(count) < sizeof(header) + payloadBytes) continue;
        packetsReceived_++;
        if (handler_) {
            std::vector<float> samples(header.sampleCount);
            std::memcpy(samples.data(), packet.data() + sizeof(header), payloadBytes);
            handler_(header, std::move(samples));
        }
    }
}


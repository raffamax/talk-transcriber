#pragma once

#include "AudioPacket.h"
#include <atomic>
#include <cstdint>
#include <thread>
#include <functional>
#include <vector>

class AudioReceiver {
public:
    using PacketHandler = std::function<void(const talk_transcriber::protocol::AudioPacketHeader&, std::vector<float>)>;
    explicit AudioReceiver(std::uint16_t port, PacketHandler handler = {});
    ~AudioReceiver();
    bool start();
    void stop();
    std::uint64_t packetsReceived() const { return packetsReceived_.load(); }
private:
    void run();
    std::uint16_t port_;
    int socket_ = -1;
    std::atomic<bool> running_{false};
    std::atomic<std::uint64_t> packetsReceived_{0};
    std::thread worker_;
    PacketHandler handler_;
};


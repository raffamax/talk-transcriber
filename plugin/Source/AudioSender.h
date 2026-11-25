#pragma once
#include <JuceHeader.h>
#include "AudioPacket.h"
#include <array>
#include <cstdint>
#include <atomic>

class AudioSender final : private juce::Thread {
public:
    AudioSender();
    ~AudioSender() override;
    void start();
    void push(const float* monoSamples, int count, double sampleRate) noexcept;
    void setChannelId(juce::String id);
    std::uint64_t packetsSent() const noexcept { return packetsSent_.load(); }
    std::uint64_t packetsDropped() const noexcept { return packetsDropped_.load(); }
    std::uint64_t packetsAttempted() const noexcept { return packetsAttempted_.load(); }
    int lastSocketError() const noexcept { return lastSocketError_.load(); }
    bool socketReady() const noexcept { return socketReady_.load(); }
private:
    void run() override;
    juce::AbstractFifo fifo_{32};
    std::array<std::array<float, talk_transcriber::protocol::kMaxSamplesPerPacket>, 32> blocks_{};
    std::array<int, 32> sizes_{};
    std::array<double, 32> rates_{};
    juce::CriticalSection metadataLock_;
    juce::String channelId_{"channel-001"};
    std::uint64_t sequence_ = 0;
    std::atomic<std::uint64_t> packetsSent_{0};
    std::atomic<std::uint64_t> packetsDropped_{0};
    std::atomic<std::uint64_t> packetsAttempted_{0};
    std::atomic<int> lastSocketError_{0};
    std::atomic<bool> socketReady_{false};
};


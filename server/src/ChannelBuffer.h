#pragma once
#include <cstddef>
#include <deque>
#include <mutex>
#include <vector>

class ChannelBuffer {
public:
    explicit ChannelBuffer(std::size_t maxSamples = 48000 * 10);
    void push(const float* samples, std::size_t count);
    std::vector<float> take(std::size_t maxSamples);
    std::size_t size() const;
private:
    const std::size_t maxSamples_;
    mutable std::mutex mutex_;
    std::deque<float> samples_;
};


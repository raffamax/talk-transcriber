#include "ChannelBuffer.h"
#include <algorithm>

ChannelBuffer::ChannelBuffer(std::size_t maxSamples) : maxSamples_(maxSamples) {}
void ChannelBuffer::push(const float* samples, std::size_t count) {
    std::lock_guard lock(mutex_);
    for (std::size_t i = 0; i < count; ++i) samples_.push_back(samples[i]);
    while (samples_.size() > maxSamples_) samples_.pop_front();
}
std::vector<float> ChannelBuffer::take(std::size_t maxSamples) {
    std::lock_guard lock(mutex_);
    const auto count = std::min(maxSamples, samples_.size());
    std::vector<float> result;
    result.reserve(count);
    for (std::size_t i = 0; i < count; ++i) { result.push_back(samples_.front()); samples_.pop_front(); }
    return result;
}
std::size_t ChannelBuffer::size() const { std::lock_guard lock(mutex_); return samples_.size(); }


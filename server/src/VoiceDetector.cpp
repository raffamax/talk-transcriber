#include "VoiceDetector.h"
#include <algorithm>
#include <cmath>

VoiceDetector::VoiceDetector() : VoiceDetector(Config{}) {}
VoiceDetector::VoiceDetector(Config config) : config_(config) { preRoll_.reserve(config_.preRollSamples); segment_.reserve(48000 * 20); }

std::vector<float> VoiceDetector::process(const float* samples, std::size_t count) {
    if (!samples || count == 0) return {};
    float peak = 0.0f;
    for (std::size_t i = 0; i < count; ++i) peak = std::max(peak, std::abs(samples[i]));
    const bool voiced = peak >= config_.threshold;
    if (!inSpeech_) {
        if (preRoll_.size() + count > config_.preRollSamples) preRoll_.erase(preRoll_.begin(), preRoll_.begin() + std::min(preRoll_.size(), count));
        preRoll_.insert(preRoll_.end(), samples, samples + count);
        if (voiced) { inSpeech_ = true; segment_ = preRoll_; preRoll_.clear(); silentSamples_ = 0; }
        return {};
    }
    segment_.insert(segment_.end(), samples, samples + count);
    if (voiced) silentSamples_ = 0; else silentSamples_ += count;
    if (silentSamples_ < config_.silenceSamples) return {};
    auto finished = std::move(segment_);
    segment_.clear(); inSpeech_ = false; silentSamples_ = 0;
    return finished;
}


#pragma once
#include <cstddef>
#include <vector>

class VoiceDetector {
public:
    struct Config { float threshold = 0.012f; std::size_t preRollSamples = 2400; std::size_t silenceSamples = 12000; };
    VoiceDetector();
    explicit VoiceDetector(Config config);
    std::vector<float> process(const float* samples, std::size_t count);
    bool inSpeech() const { return inSpeech_; }
private:
    Config config_;
    std::vector<float> preRoll_;
    std::vector<float> segment_;
    std::size_t silentSamples_ = 0;
    bool inSpeech_ = false;
};


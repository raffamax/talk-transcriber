#pragma once
#include <string>
#include <vector>

class WhisperEngine {
public:
    explicit WhisperEngine(const std::string& modelPath, bool useGpu = true);
    ~WhisperEngine();
    WhisperEngine(const WhisperEngine&) = delete;
    WhisperEngine& operator=(const WhisperEngine&) = delete;
    bool ready() const;
    std::string transcribe(const std::vector<float>& audio);
private:
#ifdef TALK_TRANSCRIBER_HAS_WHISPER
    struct whisper_context* context_ = nullptr;
#endif
};


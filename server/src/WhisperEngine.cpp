#include "WhisperEngine.h"
#include <thread>
#include <algorithm>
#ifdef TALK_TRANSCRIBER_HAS_WHISPER
#include <whisper.h>
#endif

WhisperEngine::WhisperEngine(const std::string& modelPath, bool useGpu) {
#ifdef TALK_TRANSCRIBER_HAS_WHISPER
    auto params = whisper_context_default_params();
    params.use_gpu = useGpu;
    context_ = whisper_init_from_file_with_params(modelPath.c_str(), params);
#else
    (void) modelPath; (void) useGpu;
#endif
}
WhisperEngine::~WhisperEngine() {
#ifdef TALK_TRANSCRIBER_HAS_WHISPER
    if (context_) whisper_free(context_);
#endif
}
bool WhisperEngine::ready() const {
#ifdef TALK_TRANSCRIBER_HAS_WHISPER
    return context_ != nullptr;
#else
    return false;
#endif
}
std::string WhisperEngine::transcribe(const std::vector<float>& audio) {
#ifdef TALK_TRANSCRIBER_HAS_WHISPER
    if (!context_ || audio.empty()) return {};
    auto params = whisper_full_default_params(WHISPER_SAMPLING_BEAM_SEARCH);
    params.language = "pt";
    params.translate = false;
    params.no_context = true;
    params.single_segment = true;
    params.no_timestamps = true;
    params.n_threads = static_cast<int>(std::max(2u, std::thread::hardware_concurrency()));
    params.initial_prompt = "Português brasileiro.";
    params.temperature = 0.0f;
    params.temperature_inc = 0.0f;
    params.beam_search.beam_size = 3;
    params.suppress_blank = true;
    params.suppress_nst = false;
    params.no_speech_thold = 0.6f;
    params.print_progress = false; params.print_realtime = false; params.print_timestamps = false;
    if (whisper_full(context_, params, audio.data(), static_cast<int>(audio.size())) != 0) return {};
    std::string result;
    for (int i = 0; i < whisper_full_n_segments(context_); ++i) result += whisper_full_get_segment_text(context_, i);
    return result;
#else
    (void) audio; return {};
#endif
}


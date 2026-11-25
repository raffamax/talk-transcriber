#include <iostream>
#include "AudioReceiver.h"
#include "AudioPacket.h"
#include <string>
#include "WhisperEngine.h"
#include <iomanip>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <cmath>
#include "VoiceDetector.h"
#include "WebSocketPublisher.h"

int main(int argc, char** argv) {
    std::string modelPath;
    bool useGpu = true;
    bool verbose = false;
    std::uint16_t udpPort = talk_transcriber::protocol::kDefaultPort;
    int websocketPort = 39742;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--model" && i + 1 < argc) modelPath = argv[++i];
        else if (arg == "--udp-port" && i + 1 < argc) udpPort = static_cast<std::uint16_t>(std::stoi(argv[++i]));
        else if (arg == "--ws-port" && i + 1 < argc) websocketPort = std::stoi(argv[++i]);
        else if (arg == "--cpu") useGpu = false;
        else if (arg == "--verbose") verbose = true;
    }
    WhisperEngine whisper(modelPath, useGpu);
    WebSocketPublisher publisher;
    const bool websocketReady = publisher.start(websocketPort);
    std::mutex vadMutex;
    VoiceDetector::Config defaultVad;
    std::unordered_map<std::string, VoiceDetector::Config> vadConfigs;
    std::unordered_set<std::string> vadResets;
    publisher.setVadControlHandler([&vadMutex, &defaultVad, &vadConfigs, &vadResets](const std::string& channel, float thresholdDb, int releaseMs) {
        std::lock_guard lock(vadMutex);
        auto& config = channel.empty() ? defaultVad : vadConfigs[channel];
        config.threshold = std::pow(10.0f, thresholdDb / 20.0f);
        config.silenceSamples = static_cast<std::size_t>(releaseMs * 48.0);
        if (!channel.empty()) vadResets.insert(channel);
    });
    if (!modelPath.empty()) {
        if (!whisper.ready()) { std::cerr << "Unable to load Whisper model: " << modelPath << "\n"; return 1; }
        std::cout << "Whisper model loaded (Portuguese): " << modelPath << "\n";
    }
    std::unordered_map<std::string, VoiceDetector> detectors;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> lastSeen;
    std::uint64_t packetsLogged = 0;
    AudioReceiver receiver(udpPort, [&whisper, &detectors, &lastSeen, &publisher, &vadMutex, &defaultVad, &vadConfigs, &vadResets, verbose, &packetsLogged](const auto& header, std::vector<float> samples) {
        std::string channel(reinterpret_cast<const char*>(header.channelId), strnlen(reinterpret_cast<const char*>(header.channelId), sizeof(header.channelId)));
        if (verbose && (packetsLogged++ == 0 || packetsLogged % 50 == 0))
            std::cout << "audio packet channel=" << channel << " sequence=" << header.sequence
                      << " samples=" << samples.size() << " rate=" << header.sampleRate << "\n";
        const auto now = std::chrono::steady_clock::now();
        VoiceDetector::Config config;
        {
            std::lock_guard lock(vadMutex);
            if (vadResets.erase(channel) > 0) { detectors.erase(channel); lastSeen.erase(channel); }
            config = defaultVad;
            if (const auto found = vadConfigs.find(channel); found != vadConfigs.end()) config = found->second;
        }
        for (auto seen = lastSeen.begin(); seen != lastSeen.end();) {
            if (now - seen->second > std::chrono::seconds(30)) { detectors.erase(seen->first); seen = lastSeen.erase(seen); publisher.publishStatus(whisper.ready(), detectors.size()); }
            else ++seen;
        }
        config.silenceSamples = config.silenceSamples > 0 ? config.silenceSamples : static_cast<std::size_t>(header.sampleRate * 0.15);
        auto [it, inserted] = detectors.try_emplace(channel, config);
        lastSeen[channel] = now;
        if (inserted) publisher.publishStatus(whisper.ready(), detectors.size());
        auto segment = it->second.process(samples.data(), samples.size());
        if (segment.empty() || !whisper.ready()) return;

        const int sourceRate = static_cast<int>(header.sampleRate);
        if (sourceRate <= 0) return;
        std::vector<float> audio;
        audio.reserve(static_cast<std::size_t>(segment.size()) * 16000 / sourceRate + 1);
        for (std::size_t i = 0; i < segment.size(); ++i) {
            const auto source = static_cast<std::size_t>((static_cast<double>(i) * sourceRate) / 16000);
            if (source < segment.size()) audio.push_back(segment[source]);
        }
        const auto inferenceStart = std::chrono::steady_clock::now();
        auto text = whisper.transcribe(audio);
        const auto latencyMs = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - inferenceStart).count());
        if (!text.empty()) {
            std::cout << "transcription channel=" << channel << " sequence=" << header.sequence << " latency_ms=" << latencyMs << ": " << text << "\n";
            publisher.publishFinal(channel, text, header.sequence, latencyMs);
        }
    });
    if (!receiver.start()) { std::cerr << "Unable to bind localhost UDP audio receiver\n"; return 1; }
    std::cout << "Talk Transcriber server listening on 127.0.0.1:"
              << udpPort << "\nWebSocket: " << (websocketReady ? "127.0.0.1:" + std::to_string(websocketPort) : "disabled")
              << "\nVerbose UDP logging: " << (verbose ? "on" : "off") << "\nPress Ctrl+C to stop.\n";
    publisher.publishStatus(whisper.ready(), detectors.size());
    std::cin.get();
    receiver.stop();
    publisher.stop();
    return 0;
}


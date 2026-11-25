#include "AudioPacket.h"
#include "VoiceDetector.h"
#include "ChannelBuffer.h"
#include <cassert>
#include <cmath>
#include <cstring>
#include <vector>

int main() {
    using namespace talk_transcriber::protocol;

    AudioPacketHeader packet{};
    packet.sequence = 42;
    packet.sampleRate = 48000;
    packet.channels = 1;
    packet.sampleCount = 960;
    std::memcpy(packet.channelId, "operator-a", 10);
    assert(sizeof(packet) == 40);
    assert(packet.magic == kMagic);
    assert(packet.version == kVersion);
    assert(packet.headerBytes == sizeof(AudioPacketHeader));
    assert(packet.sampleCount <= kMaxSamplesPerPacket);

    VoiceDetector detector({.threshold = 0.1f, .preRollSamples = 2, .silenceSamples = 3});
    const std::vector<float> quiet = {0.01f, 0.01f};
    const std::vector<float> voice = {0.2f, -0.3f};
    assert(detector.process(quiet.data(), quiet.size()).empty());
    assert(detector.process(voice.data(), voice.size()).empty());
    assert(detector.inSpeech());
    const std::vector<float> silence = {0.0f, 0.0f, 0.0f};
    const auto segment = detector.process(silence.data(), silence.size());
    assert(!segment.empty());
    assert(segment.size() == 5);
    assert(std::fabs(segment[0] - 0.2f) < 0.0001f);
    assert(!detector.inSpeech());

    ChannelBuffer buffer(3);
    const float samples[] = {1.0f, 2.0f, 3.0f, 4.0f};
    buffer.push(samples, 4);
    assert(buffer.size() == 3);
    const auto retained = buffer.take(3);
    assert(retained.size() == 3);
    assert(retained[0] == 2.0f && retained[2] == 4.0f);
    assert(buffer.size() == 0);

    VoiceDetector operatorA({.threshold = 0.1f, .preRollSamples = 0, .silenceSamples = 2});
    VoiceDetector operatorB({.threshold = 0.1f, .preRollSamples = 0, .silenceSamples = 2});
    const std::vector<float> aVoice = {0.3f};
    const std::vector<float> bVoice = {0.4f};
    assert(operatorA.process(aVoice.data(), aVoice.size()).empty());
    assert(!operatorB.inSpeech());
    assert(operatorB.process(bVoice.data(), bVoice.size()).empty());
    assert(operatorA.inSpeech() && operatorB.inSpeech());
    const std::vector<float> twoSilent = {0.0f, 0.0f};
    assert(!operatorA.process(twoSilent.data(), twoSilent.size()).empty());
    assert(!operatorB.process(twoSilent.data(), twoSilent.size()).empty());
    assert(!operatorA.inSpeech() && !operatorB.inSpeech());
    return 0;
}

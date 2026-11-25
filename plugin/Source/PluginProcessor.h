#pragma once
#include <JuceHeader.h>
#include "AudioSender.h"

class TalkCaptureAudioProcessor final : public juce::AudioProcessor {
public:
    TalkCaptureAudioProcessor();
    ~TalkCaptureAudioProcessor() override = default;
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override { return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet(); }
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Talk Capture"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState& getState() noexcept { return state; }
    float inputPeak() const noexcept { return inputPeak_.load(); }
    std::uint64_t packetsSent() const noexcept { return sender.packetsSent(); }
    std::uint64_t packetsDropped() const noexcept { return sender.packetsDropped(); }
    std::uint64_t packetsAttempted() const noexcept { return sender.packetsAttempted(); }
    int lastSocketError() const noexcept { return sender.lastSocketError(); }
    bool socketReady() const noexcept { return sender.socketReady(); }
    void setOperatorName(const juce::String& name) { sender.setChannelId(name.isNotEmpty() ? name : instanceId); }
private:
    juce::AudioProcessorValueTreeState state;
    AudioSender sender;
    juce::String instanceId;
    std::atomic<float> inputPeak_{0.0f};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TalkCaptureAudioProcessor)
};


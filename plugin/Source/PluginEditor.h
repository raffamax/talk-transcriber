#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class TalkCaptureAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit TalkCaptureAudioProcessorEditor(TalkCaptureAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
    bool handleHostSpaceKey();
    void timerCallback() override;
private:
    juce::Label operatorLabel_;
    juce::TextEditor operatorName_;
    juce::TextButton saveOperatorButton_{"Salvar"};
    juce::ToggleButton captureButton_{"Capture enabled"};
    juce::Label serverLabel_;
    juce::TextEditor serverAddress_;
    juce::Label signalStatus_;
    juce::Label packetStatus_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> captureAttachment_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TalkCaptureAudioProcessorEditor)
};


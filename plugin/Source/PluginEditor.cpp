#include "PluginEditor.h"
TalkCaptureAudioProcessorEditor::TalkCaptureAudioProcessorEditor(TalkCaptureAudioProcessor& p) : AudioProcessorEditor(p) {
    setSize(390, 280);
    operatorLabel_.setText("Operator", juce::dontSendNotification);
    serverLabel_.setText("Server", juce::dontSendNotification);
    addAndMakeVisible(operatorLabel_); addAndMakeVisible(operatorName_); addAndMakeVisible(saveOperatorButton_);
    addAndMakeVisible(serverLabel_); addAndMakeVisible(serverAddress_); addAndMakeVisible(captureButton_);
    addAndMakeVisible(signalStatus_); addAndMakeVisible(packetStatus_);
    signalStatus_.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    packetStatus_.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    operatorName_.setText(p.getState().state.getProperty("operatorName", "Talkback").toString());
    operatorName_.setWantsKeyboardFocus(true);
    operatorName_.setInterceptsMouseClicks(true, true);
    operatorName_.setMultiLine(false);
    operatorName_.setReturnKeyStartsNewLine(false);
    serverAddress_.setText(p.getState().state.getProperty("serverAddress", "127.0.0.1:39741").toString());
    captureAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.getState(), "captureEnabled", captureButton_);
    p.setOperatorName(operatorName_.getText());
    const auto applyOperatorName = [&p, this] {
        const auto name = operatorName_.getText().trim();
        if (name.isNotEmpty()) {
            p.getState().state.setProperty("operatorName", name, nullptr);
            p.setOperatorName(name);
        }
    };
    saveOperatorButton_.onClick = applyOperatorName;
    operatorName_.onReturnKey = [this] { operatorName_.giveAwayKeyboardFocus(); };
    serverAddress_.onTextChange = [&p, this] { p.getState().state.setProperty("serverAddress", serverAddress_.getText(), nullptr); };
    startTimerHz(5);
}
void TalkCaptureAudioProcessorEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colours::darkgrey); g.setColour(juce::Colours::white); g.setFont(22.0f); g.drawText("Talk Capture", 20, 18, 340, 30, juce::Justification::left); g.setFont(13.0f); g.drawText("Local UDP capture · pass-through enabled", 20, 50, 340, 22, juce::Justification::left); }
bool TalkCaptureAudioProcessorEditor::handleHostSpaceKey() {
    return operatorName_.isShowing() && operatorName_.hasKeyboardFocus(false)
        && operatorName_.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey, {}, ' '));
}
void TalkCaptureAudioProcessorEditor::resized() {
    operatorLabel_.setBounds(20, 82, 90, 24); operatorName_.setBounds(115, 82, 190, 24); saveOperatorButton_.setBounds(312, 82, 53, 24);
    serverLabel_.setBounds(20, 120, 90, 24); serverAddress_.setBounds(115, 120, 250, 24);
    captureButton_.setBounds(20, 165, 220, 26);
    signalStatus_.setBounds(20, 205, 350, 24); packetStatus_.setBounds(20, 232, 350, 24);
}
void TalkCaptureAudioProcessorEditor::timerCallback() {
    auto& processor = *static_cast<TalkCaptureAudioProcessor*>(getAudioProcessor());
    const auto peak = processor.inputPeak();
    const auto db = peak > 0.000001f ? juce::Decibels::gainToDecibels(peak) : -100.0f;
    signalStatus_.setText("Input: " + juce::String(db, 1) + " dBFS" + (peak > 0.012f ? " · signal detected" : " · below VAD threshold"), juce::dontSendNotification);
    packetStatus_.setText("UDP sent: " + juce::String(static_cast<juce::int64>(processor.packetsSent())) + " · attempts: " + juce::String(static_cast<juce::int64>(processor.packetsAttempted())) + " · dropped: " + juce::String(static_cast<juce::int64>(processor.packetsDropped())) + " · socket: " + (processor.socketReady() ? "ready" : "failed") + (processor.lastSocketError() ? " (errno " + juce::String(processor.lastSocketError()) + ")" : ""), juce::dontSendNotification);
}


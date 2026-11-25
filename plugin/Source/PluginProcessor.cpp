#include "PluginProcessor.h"
#include "PluginEditor.h"

TalkCaptureAudioProcessor::TalkCaptureAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "STATE", {
          std::make_unique<juce::AudioParameterBool>("captureEnabled", "Capture enabled", true)
      }),
      instanceId(juce::Uuid().toString()) {
    sender.setChannelId("Talkback");
    sender.start();
}

void TalkCaptureAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    // Pass-through only in Phase 1. No allocation or blocking work belongs here.
    juce::ScopedNoDenormals noDenormals;
    ignoreUnused(buffer);
    // The sender queue is bounded; a full queue drops capture data, never pass-through audio.
    if (buffer.getNumChannels() > 0) inputPeak_.store(buffer.getMagnitude(0, 0, buffer.getNumSamples()));
    if (buffer.getNumChannels() > 0 && state.getRawParameterValue("captureEnabled")->load() > 0.5f)
        sender.push(buffer.getReadPointer(0), buffer.getNumSamples(), getSampleRate());
}

juce::AudioProcessorEditor* TalkCaptureAudioProcessor::createEditor() { return new TalkCaptureAudioProcessorEditor(*this); }
void TalkCaptureAudioProcessor::getStateInformation(juce::MemoryBlock& dest) { if (auto xml = state.copyState().createXml()) copyXmlToBinary(*xml, dest); }
void TalkCaptureAudioProcessor::setStateInformation(const void* data, int size) { if (auto xml = getXmlFromBinary(data, size)) state.replaceState(juce::ValueTree::fromXml(*xml)); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TalkCaptureAudioProcessor(); }


cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED JUCE_DIR)
  message(FATAL_ERROR "JUCE_DIR is required. Install JUCE and configure with -DJUCE_DIR=/path/to/JUCE")
endif()

add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)

juce_add_plugin(TalkCapture
  COMPANY_NAME "Talk Transcriber"
  BUNDLE_ID com.talktranscriber.capture
  PLUGIN_MANUFACTURER_CODE Tktr
  PLUGIN_CODE TCap
  FORMATS VST3
  VST3_CAN_REPLACE_VST2 FALSE
  PRODUCT_NAME "Talk Capture"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(TalkCapture)

target_sources(TalkCapture PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp
  Source/AudioSender.cpp
)
target_include_directories(TalkCapture PRIVATE Source)
target_include_directories(TalkCapture PRIVATE ${CMAKE_SOURCE_DIR}/protocol)
target_compile_features(TalkCapture PRIVATE cxx_std_20)
target_compile_definitions(TalkCapture_VST3 PRIVATE JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(TalkCapture PRIVATE juce::juce_audio_utils)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  include(cmake/Vst3Keyboard.cmake)
endif()


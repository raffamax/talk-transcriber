add_executable(talk-transcriber-tests
  protocol_and_vad.cpp
  ${CMAKE_SOURCE_DIR}/server/src/VoiceDetector.cpp
  ${CMAKE_SOURCE_DIR}/server/src/AudioBuffer.cpp
)
target_compile_features(talk-transcriber-tests PRIVATE cxx_std_20)
target_include_directories(talk-transcriber-tests PRIVATE
  ${CMAKE_SOURCE_DIR}/protocol
  ${CMAKE_SOURCE_DIR}/server/src
)
add_test(NAME talk-transcriber-tests COMMAND talk-transcriber-tests)

add_executable(talk-transcriber-server
  src/main.cpp
  src/AudioReceiver.cpp
  src/AudioBuffer.cpp
  src/VoiceDetector.cpp
  src/WhisperEngine.cpp
  src/WebSocketServer.cpp
)

target_compile_features(talk-transcriber-server PRIVATE cxx_std_20)
target_include_directories(talk-transcriber-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(talk-transcriber-server PRIVATE ${CMAKE_SOURCE_DIR}/protocol)

if(TALK_TRANSCRIBER_HAS_WHISPER)
  target_compile_definitions(talk-transcriber-server PRIVATE TALK_TRANSCRIBER_HAS_WHISPER=1)
  target_link_libraries(talk-transcriber-server PRIVATE whisper)
endif()
if(TALK_TRANSCRIBER_HAS_WEBSOCKET)
  target_compile_definitions(talk-transcriber-server PRIVATE TALK_TRANSCRIBER_HAS_WEBSOCKET=1)
  target_include_directories(talk-transcriber-server PRIVATE ${IXWEBSOCKET_DIR}/ixwebsocket)
  target_link_libraries(talk-transcriber-server PRIVATE ixwebsocket::ixwebsocket)
endif()

if(APPLE)
  target_compile_definitions(talk-transcriber-server PRIVATE TALK_TRANSCRIBER_APPLE=1)
endif()


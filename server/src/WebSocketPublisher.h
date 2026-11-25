#pragma once
#include <string>
#include <cstdint>
#include <functional>

class WebSocketPublisher {
public:
    WebSocketPublisher();
    ~WebSocketPublisher();
    bool start(int port = 39742);
    void stop();
    void publishStatus(bool whisperReady, std::size_t channels);
    void setVadControlHandler(std::function<void(const std::string&, float, int)> handler);
    void publishFinal(const std::string& channelId, const std::string& text, std::uint64_t sequence, std::uint64_t latencyMs = 0);
private:
    struct Impl;
    Impl* impl_ = nullptr;
};


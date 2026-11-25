#pragma once
#include <string>
#include <cstdint>
#include <functional>
#include <vector>

class WebSocketPublisher {
public:
    struct ChannelStatus { std::string id; std::string name; };
    WebSocketPublisher();
    ~WebSocketPublisher();
    bool start(int port = 39742);
    void stop();
    void publishStatus(bool whisperReady, const std::vector<ChannelStatus>& channels);
    void setVadControlHandler(std::function<void(const std::string&, float, int)> handler);
    void publishFinal(const std::string& channelId, const std::string& channelName, const std::string& text,
                      std::uint64_t sequence, std::uint64_t latencyMs = 0);
private:
    struct Impl;
    Impl* impl_ = nullptr;
};


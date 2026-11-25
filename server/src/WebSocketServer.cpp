#include "WebSocketPublisher.h"
#include <chrono>
#include <sstream>
#include <mutex>
#include <iomanip>
#include <memory>
#include <cmath>
#include <algorithm>
#ifdef TALK_TRANSCRIBER_HAS_WEBSOCKET
#include <IXWebSocketServer.h>
#endif

struct WebSocketPublisher::Impl {
#ifdef TALK_TRANSCRIBER_HAS_WEBSOCKET
    std::unique_ptr<ix::WebSocketServer> server;
    std::string lastStatus;
    std::function<void(const std::string&, float, int)> vadHandler;
#endif
    std::mutex mutex;
};
static std::string jsonEscape(const std::string& value) {
    std::ostringstream out;
    for (const unsigned char c : value) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: if (c < 0x20) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c); else out << c;
        }
    }
    return out.str();
}
static bool jsonNumber(const std::string& json, const char* key, double& value) {
    const auto marker = std::string("\"") + key + "\":";
    const auto pos = json.find(marker);
    if (pos == std::string::npos) return false;
    try { value = std::stod(json.substr(pos + marker.size())); return true; } catch (...) { return false; }
}
static std::string jsonString(const std::string& json, const char* key) {
    const auto marker = std::string("\"") + key + "\":\"";
    const auto pos = json.find(marker);
    if (pos == std::string::npos) return {};
    const auto start = pos + marker.size();
    const auto end = json.find('"', start);
    return end == std::string::npos ? std::string{} : json.substr(start, end - start);
}
WebSocketPublisher::WebSocketPublisher() : impl_(new Impl) {}
WebSocketPublisher::~WebSocketPublisher() { stop(); delete impl_; }
bool WebSocketPublisher::start(int port) {
#ifdef TALK_TRANSCRIBER_HAS_WEBSOCKET
    impl_->server = std::make_unique<ix::WebSocketServer>(port, "0.0.0.0");
    impl_->server->makeBroadcastServer();
    impl_->server->setOnConnectionCallback([impl = impl_](std::weak_ptr<ix::WebSocket> weak, std::shared_ptr<ix::ConnectionState>) {
        if (auto client = weak.lock()) {
            client->setOnMessageCallback([impl, weak](const ix::WebSocketMessagePtr& message) {
                double thresholdDb = 0.0, releaseMs = 0.0;
                if (message && message->str.find("vad.config") != std::string::npos && jsonNumber(message->str, "thresholdDb", thresholdDb) && jsonNumber(message->str, "releaseMs", releaseMs) && impl->vadHandler)
                    impl->vadHandler(jsonString(message->str, "channelId"), static_cast<float>(thresholdDb), static_cast<int>(std::clamp(releaseMs, 20.0, 5000.0)));
                if (auto connected = weak.lock()) {
                    std::lock_guard lock(impl->mutex);
                    if (!impl->lastStatus.empty()) connected->sendText(impl->lastStatus);
                }
            });
            std::lock_guard lock(impl->mutex);
            if (!impl->lastStatus.empty()) client->sendText(impl->lastStatus);
        }
    });
    return impl_->server->listenAndStart();
#else
    (void) port; return false;
#endif
}
void WebSocketPublisher::setVadControlHandler(std::function<void(const std::string&, float, int)> handler) {
#ifdef TALK_TRANSCRIBER_HAS_WEBSOCKET
    std::lock_guard lock(impl_->mutex);
    impl_->vadHandler = std::move(handler);
#else
    (void) handler;
#endif
}
void WebSocketPublisher::stop() {
#ifdef TALK_TRANSCRIBER_HAS_WEBSOCKET
    std::lock_guard lock(impl_->mutex);
    if (impl_->server) impl_->server->stop();
#endif
}
void WebSocketPublisher::publishStatus(bool whisperReady, const std::vector<ChannelStatus>& channels) {
#ifdef TALK_TRANSCRIBER_HAS_WEBSOCKET
    std::lock_guard lock(impl_->mutex);
    std::ostringstream json;
    json << "{\"version\":1,\"type\":\"server.status\",\"whisperReady\":" << (whisperReady ? "true" : "false")
         << ",\"channels\":" << channels.size() << ",\"activeChannels\":[";
    for (std::size_t i = 0; i < channels.size(); ++i) {
        if (i > 0) json << ',';
        json << "{\"id\":\"" << jsonEscape(channels[i].id) << "\",\"name\":\"" << jsonEscape(channels[i].name) << "\"}";
    }
    json << "]}";
    impl_->lastStatus = json.str();
    if (impl_->server) for (const auto& client : impl_->server->getClients()) client->sendText(impl_->lastStatus);
#else
    (void) whisperReady; (void) channels;
#endif
}
void WebSocketPublisher::publishFinal(const std::string& channelId, const std::string& channelName, const std::string& text,
                                      std::uint64_t sequence, std::uint64_t latencyMs) {
#ifdef TALK_TRANSCRIBER_HAS_WEBSOCKET
    std::lock_guard lock(impl_->mutex);
    std::ostringstream json;
    json << "{\"version\":1,\"type\":\"transcription.final\",\"channelId\":\"" << jsonEscape(channelId)
         << "\",\"channelName\":\"" << jsonEscape(channelName) << "\",\"sequence\":" << sequence
         << ",\"latencyMs\":" << latencyMs << ",\"text\":\"" << jsonEscape(text) << "\"}";
    if (impl_->server) for (const auto& client : impl_->server->getClients()) client->sendText(json.str());
#else
    (void) channelId; (void) channelName; (void) text; (void) sequence; (void) latencyMs;
#endif
}


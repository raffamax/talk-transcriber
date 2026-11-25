#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace talk_transcriber::protocol {

inline constexpr std::uint32_t kMagic = 0x54544150; // "TTAP"
inline constexpr std::uint16_t kLegacyVersion = 1;
inline constexpr std::uint16_t kPreviousVersion = 2;
inline constexpr std::uint16_t kVersion = 3;
inline constexpr std::uint16_t kDefaultPort = 39741;
inline constexpr std::size_t kMaxSamplesPerPacket = 960; // 20 ms at 48 kHz

#pragma pack(push, 1)
struct AudioPacketHeader {
    std::uint32_t magic = kMagic;
    std::uint16_t version = kVersion;
    std::uint16_t headerBytes = sizeof(AudioPacketHeader);
    char channelName[64]{};
    std::uint64_t sequence = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t channels = 0;
    std::uint16_t sampleCount = 0;
    std::uint8_t instanceId[16]{};
};

struct LegacyAudioPacketHeader {
    std::uint32_t magic = kMagic;
    std::uint16_t version = kLegacyVersion;
    std::uint16_t headerBytes = sizeof(LegacyAudioPacketHeader);
    char channelName[16]{};
    std::uint64_t sequence = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t channels = 0;
    std::uint16_t sampleCount = 0;
};

struct PreviousAudioPacketHeader : LegacyAudioPacketHeader {
    std::uint8_t instanceId[16]{};
};
#pragma pack(pop)

inline constexpr std::size_t kLegacyHeaderBytes = sizeof(LegacyAudioPacketHeader);
inline constexpr std::size_t kPreviousHeaderBytes = sizeof(PreviousAudioPacketHeader);
static_assert(kLegacyHeaderBytes == 40);
static_assert(kPreviousHeaderBytes == 56);
static_assert(sizeof(AudioPacketHeader) == 104);

inline bool readAudioPacketHeader(const std::byte* packet, std::size_t size, AudioPacketHeader& header) {
    if (size < kLegacyHeaderBytes) return false;
    header = {};
    std::uint16_t version = 0;
    std::memcpy(&version, packet + sizeof(std::uint32_t), sizeof(version));
    std::size_t headerBytes = 0;
    if (version == kLegacyVersion) {
        LegacyAudioPacketHeader legacy{};
        std::memcpy(&legacy, packet, sizeof(legacy));
        header.magic = legacy.magic;
        header.version = legacy.version;
        header.headerBytes = legacy.headerBytes;
        std::memcpy(header.channelName, legacy.channelName, sizeof(legacy.channelName));
        header.sequence = legacy.sequence;
        header.sampleRate = legacy.sampleRate;
        header.channels = legacy.channels;
        header.sampleCount = legacy.sampleCount;
        headerBytes = sizeof(legacy);
    } else if (version == kPreviousVersion) {
        PreviousAudioPacketHeader previous{};
        if (size < sizeof(previous)) return false;
        std::memcpy(&previous, packet, sizeof(previous));
        header.magic = previous.magic;
        header.version = previous.version;
        header.headerBytes = previous.headerBytes;
        std::memcpy(header.channelName, previous.channelName, sizeof(previous.channelName));
        header.sequence = previous.sequence;
        header.sampleRate = previous.sampleRate;
        header.channels = previous.channels;
        header.sampleCount = previous.sampleCount;
        std::memcpy(header.instanceId, previous.instanceId, sizeof(previous.instanceId));
        headerBytes = sizeof(previous);
    } else if (version == kVersion) {
        if (size < sizeof(header)) return false;
        std::memcpy(&header, packet, sizeof(header));
        headerBytes = sizeof(header);
    } else return false;
    if (header.magic != kMagic || header.headerBytes != headerBytes || header.sampleCount > kMaxSamplesPerPacket) return false;
    if (size < headerBytes + static_cast<std::size_t>(header.sampleCount) * sizeof(float)) return false;
    return true;
}

} // namespace talk_transcriber::protocol


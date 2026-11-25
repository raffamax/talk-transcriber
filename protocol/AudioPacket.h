#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace talk_transcriber::protocol {

inline constexpr std::uint32_t kMagic = 0x54544150; // "TTAP"
inline constexpr std::uint16_t kLegacyVersion = 1;
inline constexpr std::uint16_t kVersion = 2;
inline constexpr std::uint16_t kDefaultPort = 39741;
inline constexpr std::size_t kMaxSamplesPerPacket = 960; // 20 ms at 48 kHz

#pragma pack(push, 1)
struct AudioPacketHeader {
    std::uint32_t magic = kMagic;
    std::uint16_t version = kVersion;
    std::uint16_t headerBytes = sizeof(AudioPacketHeader);
    char channelName[16]{};
    std::uint64_t sequence = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t channels = 0;
    std::uint16_t sampleCount = 0;
    std::uint8_t instanceId[16]{};
};
#pragma pack(pop)

// Version 1 ends immediately before the instance ID.
inline constexpr std::size_t kLegacyHeaderBytes = offsetof(AudioPacketHeader, instanceId);
static_assert(kLegacyHeaderBytes == 40);
static_assert(sizeof(AudioPacketHeader) == 56);

inline bool readAudioPacketHeader(const std::byte* packet, std::size_t size, AudioPacketHeader& header) {
    if (size < kLegacyHeaderBytes) return false;
    header = {};
    std::memcpy(&header, packet, kLegacyHeaderBytes);
    if (header.magic != kMagic || header.sampleCount > kMaxSamplesPerPacket) return false;

    std::size_t headerBytes;
    if (header.version == kLegacyVersion) headerBytes = kLegacyHeaderBytes;
    else if (header.version == kVersion) headerBytes = sizeof(header);
    else return false;

    if (header.headerBytes != headerBytes || size < headerBytes + header.sampleCount * sizeof(float)) return false;
    if (header.version == kVersion) std::memcpy(header.instanceId, packet + kLegacyHeaderBytes, sizeof(header.instanceId));
    return true;
}

} // namespace talk_transcriber::protocol


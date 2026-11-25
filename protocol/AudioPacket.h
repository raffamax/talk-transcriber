#pragma once

#include <cstdint>
#include <cstddef>

namespace talk_transcriber::protocol {

inline constexpr std::uint32_t kMagic = 0x54544150; // "TTAP"
inline constexpr std::uint16_t kVersion = 1;
inline constexpr std::uint16_t kDefaultPort = 39741;
inline constexpr std::size_t kMaxSamplesPerPacket = 960; // 20 ms at 48 kHz

#pragma pack(push, 1)
struct AudioPacketHeader {
    std::uint32_t magic = kMagic;
    std::uint16_t version = kVersion;
    std::uint16_t headerBytes = sizeof(AudioPacketHeader);
    std::uint8_t channelId[16]{};
    std::uint64_t sequence = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t channels = 0;
    std::uint16_t sampleCount = 0;
};
#pragma pack(pop)

static_assert(sizeof(AudioPacketHeader) == 40);

} // namespace talk_transcriber::protocol


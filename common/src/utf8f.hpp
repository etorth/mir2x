#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <string_view>
#include <utf8.h>

namespace utf8f
{
    std::string code2str(uint32_t); // u32 code point to single-utf8-char-string

    uint32_t str2code(const char *);            // pick the first utf8-char-string to u32 code point
    uint32_t str2code(const char *, size_t &);  // pick the first utf8-char-string to u32 code point from provided position, update position if succeeds

    uint32_t str2code(const std::string);
    uint32_t str2code(const std::string, size_t &);

    uint32_t str2code(const std::string_view &);
    uint32_t str2code(const std::string_view &, size_t &);

    std::string peekFirst(const char *); // peek first utf8-char-string
    std::string peekFirst(const std::string &);
    std::string peekFirst(const std::string_view &);

    std::string peekLast(const char *); // peek last utf8-char-string
    std::string peekLast(const std::string &);
    std::string peekLast(const std::string_view &);

    constexpr uint16_t buildTTFIndex(uint8_t fontIndex, uint8_t fontSize)
    {
        return (static_cast<uint16_t>(fontIndex) << 8) | fontSize;
    }

    constexpr std::tuple<uint8_t, uint8_t> extractTTFIndex(uint16_t ttfIndex)
    {
        return
        {
            static_cast<uint8_t>(ttfIndex >> 8),
            static_cast<uint8_t>(ttfIndex >> 0),
        };
    }

    constexpr uint64_t buildU64Key(uint8_t font, uint8_t fontSize, uint8_t fontStyle, uint32_t codePoint)
    {
        return (static_cast<uint64_t>(font     ) << 48)
             + (static_cast<uint64_t>(fontSize ) << 40)
             + (static_cast<uint64_t>(fontStyle) << 32)
             + (static_cast<uint64_t>(codePoint) <<  0);
    }

    constexpr std::tuple<uint8_t, uint8_t, uint8_t, uint32_t> extractU64Key(uint64_t u64Key)
    {
        return
        {
            static_cast<uint8_t> (u64Key >> 48),
            static_cast<uint8_t> (u64Key >> 40),
            static_cast<uint8_t> (u64Key >> 32),
            static_cast<uint32_t>(u64Key >>  0),
        };
    }

    constexpr uint32_t fontInfoFromU64Key(uint64_t u64Key)
    {
        return static_cast<uint32_t>(u64Key >> 32);
    }

    constexpr uint32_t codePointFromU64Key(uint64_t u64Key)
    {
        return static_cast<uint32_t>(u64Key);
    }

    std::vector<int> buildUTF8Off(const char *);

    bool valid(const std::string &);
    std::string toupper(std::string);
}

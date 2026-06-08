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

    uint32_t str2code(std::string_view);
    uint32_t str2code(std::string_view, size_t &);

    uint32_t    peekUTF8Code(const char *, const char * = nullptr); // pick one UTF-8 character and return as code point
    std::string peekUTF8Str (const char *, const char * = nullptr); // pick one UTF-8 character and return as string

    std::vector<int> buildUTF8Off(const char *);

    constexpr uint64_t buildU64Key(uint8_t font, uint8_t fontSize, uint8_t fontStyle, uint32_t utf8Code)
    {
        return ((uint64_t)(font) << 48) + ((uint64_t)(fontSize) << 40) + ((uint64_t)(fontStyle) << 32) + (uint64_t)(utf8Code);
    }

    bool valid(const std::string &);
    std::string toupper(std::string);
}

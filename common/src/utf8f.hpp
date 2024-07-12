#pragma once
#include <vector>
#include <cstdint>

namespace utf8f
{
    uint32_t peekUTF8Code(const char *);

    std::vector<int> buildUTF8Off(const char *);

    constexpr uint64_t buildU64Key(uint8_t font, uint8_t fontSize, uint8_t fontStyle, uint32_t utf8Code)
    {
        return ((uint64_t)(font) << 48) + ((uint64_t)(fontSize) << 40) + ((uint64_t)(fontStyle) << 32) + (uint64_t)(utf8Code);
    }

    bool valid(const std::string &);
    std::string toupper(std::string);
}

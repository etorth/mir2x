#pragma once
#include <string>
#include <cstdint>

namespace aesf
{
    std::string encrypt(const char *, const char *, uint64_t);
    std::string decrypt(const char *, const char *, uint64_t);
}

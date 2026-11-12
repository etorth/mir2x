#include <charconv>
#include <system_error>
#include "totype.hpp"
#include "parserf.hpp"

template<typename T> static std::optional<T> parseValid(std::string_view s, bool (*fnValid)(T))
{
    T result{};
    if(const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), result, 10); ec == std::errc() && ptr == s.data() + s.size() && fnValid(result)){
        return result;
    }
    return std::nullopt;
}

namespace parserf
{
    std::optional<uint64_t> price(std::string_view s)
    {
        return parseValid<uint64_t>(s, valid_price<uint64_t>);
    }

    std::optional<uint64_t> price(std::u8string_view s)
    {
        return price(as_sv(s.data(), s.size()));
    }

    std::optional<uint32_t> level(std::string_view s)
    {
        return parseValid<uint32_t>(s, valid_level<uint32_t>);
    }

    std::optional<uint32_t> level(std::u8string_view s)
    {
        return level(as_sv(s.data(), s.size()));
    }

    std::optional<size_t> packcount(std::string_view s)
    {
        return parseValid<size_t>(s, valid_packcount<size_t>);
    }

    std::optional<size_t> packcount(std::u8string_view s)
    {
        return packcount(as_sv(s.data(), s.size()));
    }

    std::optional<size_t> itemcount(std::string_view s)
    {
        return parseValid<size_t>(s, valid_itemcount<size_t>);
    }

    std::optional<size_t> itemcount(std::u8string_view s)
    {
        return itemcount(as_sv(s.data(), s.size()));
    }
}

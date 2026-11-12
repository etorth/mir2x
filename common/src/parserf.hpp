#pragma once
#include <cstdint>
#include <cstddef>
#include <utility>
#include <optional>
#include <concepts>
#include <string_view>
#include "sysconst.hpp"

namespace parserf
{
    template<std::integral T> constexpr bool valid_price    (T price){ return std::cmp_greater_equal(price, 1) && std::cmp_less_equal(price, SYS_MAXPRICE      ); }
    template<std::integral T> constexpr bool valid_level    (T level){ return std::cmp_greater_equal(level, 1) && std::cmp_less_equal(level, SYS_MAXLEVEL      ); }
    template<std::integral T> constexpr bool valid_packcount(T count){ return std::cmp_greater_equal(count, 1) && std::cmp_less_equal(count, SYS_INVGRIDMAXHOLD); }
    template<std::integral T> constexpr bool valid_itemcount(T count){ return std::cmp_greater_equal(count, 1) && std::cmp_less_equal(count, SYS_MAXITEMCOUNT  ); }

    template<std::integral T, std::integral U> constexpr std::optional<uint64_t> total_price(T unit, U count)
    {
        if(!valid_price(unit) || !valid_itemcount(count)){
            return std::nullopt;
        }

        const auto unitPrice = static_cast<uint64_t>(unit);
        const auto itemCount = static_cast<uint64_t>(count);

        if(unitPrice > SYS_MAXPRICE / itemCount){
            return std::nullopt;
        }
        return unitPrice * itemCount;
    }

    std::optional<uint64_t> price(std::string_view);
    std::optional<uint64_t> price(std::u8string_view);

    std::optional<uint32_t> level(std::string_view);
    std::optional<uint32_t> level(std::u8string_view);

    std::optional<size_t> packcount(std::string_view);
    std::optional<size_t> packcount(std::u8string_view);

    std::optional<size_t> itemcount(std::string_view);
    std::optional<size_t> itemcount(std::u8string_view);
}

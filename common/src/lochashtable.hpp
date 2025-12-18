#pragma once
#include <utility>
#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include "totype.hpp"
#include "fflerror.hpp"

struct LocHashHelper
{
    size_t operator() (const std::tuple<int, int> &arg) const
    {
        return to_uz((to_u64(std::get<1>(arg)) << 32) | to_u64(std::get<0>(arg)));
    }
};

/*                */ using LocHashSet   = std::unordered_set<std::tuple<int, int>,    LocHashHelper>;
template<typename T> using LocHashTable = std::unordered_map<std::tuple<int, int>, T, LocHashHelper>;

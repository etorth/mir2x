#pragma once
#include <cstring>
#include <string>
#include <stdexcept>
#include "strf.hpp"

#define fflerror(...) std::runtime_error(str_ffl() + ": " + str_printf(__VA_ARGS__))

inline std::string _fflerror_helper(size_t index)
{
    return str_printf("[%zu]: NA", index);
}

template<typename T> std::string _fflerror_helper(size_t index, const T & t)
{
    return str_printf("[%zu]: %s", index, str_any(t).c_str());
}

template<typename T, typename ... Args> std::string _fflerror_helper(size_t index, const T & t, Args && ... args)
{
    return _fflerror_helper(index, t) + ", " + _fflerror_helper(index + 1, std::forward<Args>(args)...);
}

template<typename ... Args> constexpr size_t _fflerror_count_helper(Args && ...)
{
    return sizeof...(Args);
}

#define fflreach() fflerror("fflreach")
#define fflvalue(...) fflerror("%s", _fflerror_helper(0, __VA_ARGS__).c_str())
#define fflassert(cond, ...) do{if(cond){}else{throw fflerror("assertion failed: %s%s", #cond, (_fflerror_count_helper(__VA_ARGS__) == 0) ? "" : (std::string(", ") + _fflerror_helper(0, ##__VA_ARGS__)).c_str());}}while(0)

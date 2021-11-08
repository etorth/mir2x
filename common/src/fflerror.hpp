#pragma once
#include <sstream>
#include <stdexcept>
#include "strf.hpp"

#define fflerror(...) std::runtime_error(str_ffl() + ": " + str_printf(__VA_ARGS__))
#define fflassert(...) do{if(__VA_ARGS__){}else{throw fflerror("assertion failed: %s", #__VA_ARGS__);}}while(0)

inline std::string _ffl_bad_value_helper(size_t index, const char *s)
{
    if(!s){
        return str_printf("[%zu]: (null)", index);
    }
    else if(!s[0]){
        return str_printf("[%zu]: (empty)", index);
    }
    else{
        return str_printf("[%zu]: \"%s\"", index, s);
    }
}

inline std::string _ffl_bad_value_helper(size_t index, const std::string &s)
{
    return _ffl_bad_value_helper(index, s.c_str());
}

inline std::string _ffl_bad_value_helper(size_t index, const std::u8string &s)
{
    return _ffl_bad_value_helper(index, (const char *)(s.c_str()));
}

inline std::string _ffl_bad_value_helper(size_t index, const char8_t *s)
{
    return _ffl_bad_value_helper(index, (const char *)(s));
}

inline std::string _ffl_bad_value_helper(size_t index, char ch)
{
    if(ch == '\0'){
        return str_printf("[%zu]: \'\\0\'", index);
    }
    else{
        return str_printf("[%zu]: \'%c\'", index, ch);
    }
}

template<typename T> std::string _ffl_bad_value_helper(size_t index, const T & t)
{
    return str_printf("[%zu]: %s", index, dynamic_cast<const std::stringstream &>(std::stringstream() << t).str().c_str());
}

template<typename T, typename ... Args> std::string _ffl_bad_value_helper(size_t index, const T & t, Args && ... args)
{
    return _ffl_bad_value_helper(index, t) + ", " + _ffl_bad_value_helper(index + 1, std::forward<Args>(args)...);
}

#define bad_reach() fflerror("bad reach")
#define bad_value(...) fflerror("bad value: %s", _ffl_bad_value_helper(0, __VA_ARGS__).c_str())

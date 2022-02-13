#pragma once
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include "strf.hpp"

#define fflerror(...) std::runtime_error(str_ffl() + ": " + str_printf(__VA_ARGS__))

inline std::string _ffl_bad_value_type_helper(const char *s)
{
    if(!s){
        return "(null)";
    }
    else if(!s[0]){
        return "(empty)";
    }
    else{
        return str_printf("\"%s\"", s);
    }
}

inline std::string _ffl_bad_value_type_helper(const char8_t *s)
{
    return _ffl_bad_value_type_helper((const char *)(s));
}

inline std::string _ffl_bad_value_type_helper(const std::string &s)
{
    return _ffl_bad_value_type_helper(s.c_str());
}

inline std::string _ffl_bad_value_type_helper(const std::u8string &s)
{
    return _ffl_bad_value_type_helper((const char *)(s.c_str()));
}

inline std::string _ffl_bad_value_type_helper(char ch)
{
    if(ch == '\0'){
        return "\'\\0\'";
    }
    else{
        return str_printf("\'%c\'", ch);
    }
}

inline std::string _ffl_bad_value_type_helper(bool b)
{
    return b ? "true" : "false";
}


template<typename T> std::string _ffl_bad_value_type_helper(const std::set<T> &);
template<typename T> std::string _ffl_bad_value_type_helper(const std::unordered_set<T> &);

template<typename K, typename V> std::string _ffl_bad_value_type_helper(const std::map<K, V> &);
template<typename K, typename V> std::string _ffl_bad_value_type_helper(const std::unordered_map<K, V> &);

template<typename T> std::string _ffl_bad_value_type_helper(const std::vector<T> &);
template<typename U, typename V> std::string _ffl_bad_value_type_helper(const std::pair<U, V> &);


template<typename T> std::string _ffl_bad_value_type_helper(const T &t)
{
    return dynamic_cast<const std::stringstream &>(std::stringstream() << t).str();
}

template<typename T> std::string _ffl_bad_value_type_helper(const std::set<T> &s)
{
    std::string result = "{";
    for(const auto &t: s){
        if(result.size() > 1){
            result += ",";
        }
        result += _ffl_bad_value_type_helper(t);
    }
    return result + "}";
}

template<typename T> std::string _ffl_bad_value_type_helper(const std::unordered_set<T> &s)
{
    std::string result = "{";
    for(const auto &t: s){
        if(result.size() > 1){
            result += ",";
        }
        result += _ffl_bad_value_type_helper(t);
    }
    return result + "}";
}

template<typename K, typename V> std::string _ffl_bad_value_type_helper(const std::map<K, V> &m)
{
    std::string result = "{";
    for(const auto &t: m){
        if(result.size() > 1){
            result += ",";
        }
        result += _ffl_bad_value_type_helper(t);
    }
    return result + "}";
}

template<typename K, typename V> std::string _ffl_bad_value_type_helper(const std::unordered_map<K, V> &m)
{
    std::string result = "{";
    for(const auto &t: m){
        if(result.size() > 1){
            result += ",";
        }
        result += _ffl_bad_value_type_helper(t);
    }
    return result + "}";
}

template<typename T> std::string _ffl_bad_value_type_helper(const std::vector<T> &v)
{
    std::string result = "{";
    for(const auto &t: v){
        if(result.size() > 1){
            result += ",";
        }
        result += _ffl_bad_value_type_helper(t);
    }
    return result + "}";
}

template<typename K, typename V> std::string _ffl_bad_value_type_helper(const std::pair<K, V> &p)
{
    return std::string("{") + _ffl_bad_value_type_helper(p.first) + "," + _ffl_bad_value_type_helper(p.second) + "}";
}

inline std::string _ffl_bad_value_helper(size_t index)
{
    return str_printf("[%zu]: NA", index);
}

template<typename T> std::string _ffl_bad_value_helper(size_t index, const T & t)
{
    return str_printf("[%zu]: %s", index, _ffl_bad_value_type_helper(t).c_str());
}

template<typename T, typename ... Args> std::string _ffl_bad_value_helper(size_t index, const T & t, Args && ... args)
{
    return _ffl_bad_value_helper(index, t) + ", " + _ffl_bad_value_helper(index + 1, std::forward<Args>(args)...);
}

template<typename ... Args> constexpr size_t _ffl_va_count_helper(Args && ...)
{
    return sizeof...(Args);
}

#define fflreach() fflerror("fflreach")
#define fflvalue(...) fflerror("%s", _ffl_bad_value_helper(0, __VA_ARGS__).c_str())
#define fflassert(cond, ...) do{if(cond){}else{throw fflerror("assertion failed: %s%s", #cond, (_ffl_va_count_helper(__VA_ARGS__) == 0) ? "" : (std::string(", ") + _ffl_bad_value_helper(0, ##__VA_ARGS__)).c_str());}}while(0)

#pragma once
#include <span>
#include <cmath>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include "staticbuffer.hpp"
#include "conceptf.hpp"

inline auto to_d    (auto x){ return static_cast<               int>(x); }
inline auto to_u    (auto x){ return static_cast<      unsigned int>(x); }
inline auto to_f    (auto x){ return static_cast<             float>(x); }
inline auto to_df   (auto x){ return static_cast<            double>(x); }
inline auto to_lld  (auto x){ return static_cast<         long long>(x); }
inline auto to_llu  (auto x){ return static_cast<unsigned long long>(x); }
inline auto to_uz   (auto x){ return static_cast<            size_t>(x); }
inline auto to_zu   (auto x){ return static_cast<            size_t>(x); }
inline auto to_i8   (auto x){ return static_cast<            int8_t>(x); }
inline auto to_i16  (auto x){ return static_cast<           int16_t>(x); }
inline auto to_i32  (auto x){ return static_cast<           int32_t>(x); }
inline auto to_i64  (auto x){ return static_cast<           int64_t>(x); }
inline auto to_u8   (auto x){ return static_cast<           uint8_t>(x); }
inline auto to_u16  (auto x){ return static_cast<          uint16_t>(x); }
inline auto to_u32  (auto x){ return static_cast<          uint32_t>(x); }
inline auto to_u64  (auto x){ return static_cast<          uint64_t>(x); }
inline auto to_cvptr(auto x){ return static_cast<      const void *>(x); }

template<typename T, typename F> static T check_cast(F from)
{
    auto to = static_cast<T>(from);
    if(static_cast<F>(to) != from){
        throw std::runtime_error("cast fails to preserve original value");
    }
    return to;
}

int to_dround(std::floating_point auto x)
{
    return check_cast<int>(std::lround(x));
}

template<typename T> T as_type(const void *buf, size_t bufSize = sizeof(T))
{
    static_assert(std::is_trivially_copyable_v<T>);

    T t;
    if(bufSize >= sizeof(t)){
        std::memcpy(&t, buf, sizeof(t));
    }
    else{
        std::memcpy(&t, buf, bufSize);
        std::memset(reinterpret_cast<uint8_t *>(&t) + bufSize, 0, sizeof(T) - bufSize);
    }
    return t;
}

inline std::string_view as_sv(const void *buf, size_t bufSize)
{
    return std::string_view(reinterpret_cast<const char *>(buf), bufSize);
}

inline std::u8string_view as_u8sv(const void *buf, size_t bufSize)
{
    return std::u8string_view(reinterpret_cast<const char8_t *>(buf), bufSize);
}

inline std::string_view as_sv(const conceptf::TriviallyCopyable auto &t)
{
    return as_sv(&t, sizeof(t));
}

inline std::u8string_view as_u8sv(const conceptf::TriviallyCopyable auto &t)
{
    return as_u8sv(&t, sizeof(t));
}

inline uint16_t as_u16(const void *buf, size_t bufSize = 2) { return as_type<uint16_t>(buf, bufSize); }
inline uint32_t as_u32(const void *buf, size_t bufSize = 4) { return as_type<uint32_t>(buf, bufSize); }
inline uint64_t as_u64(const void *buf, size_t bufSize = 8) { return as_type<uint64_t>(buf, bufSize); }

template<typename T> std::span<      T> as_span(      T *data, size_t size) { return std::span<      T>(data, size); }
template<typename T> std::span<const T> as_span(const T *data, size_t size) { return std::span<const T>(data, size); }

template<typename T, typename... Args> std::span<      T> as_span(      std::vector<T, Args...> &v){ return std::span<      T>(v.data(), v.size()); }
template<typename T, typename... Args> std::span<const T> as_span(const std::vector<T, Args...> &v){ return std::span<const T>(v.data(), v.size()); }

inline const char * to_cstr(const char *s)
{
    if(s == nullptr){
        return "(null)";
    }
    else if(s[0] == '\0'){
        return "(empty)";
    }
    else{
        return s;
    }
}

inline const char *to_cstr(const unsigned char *s)
{
    return reinterpret_cast<const char *>(s);
}

inline const char *to_cstr(const char8_t *s)
{
    return reinterpret_cast<const char *>(s);
}

inline const char *to_cstr(const std::string &s)
{
    return to_cstr(s.c_str());
}

inline const char *to_cstr(const std::u8string &s)
{
    return to_cstr(s.c_str());
}

inline const char *to_cstr(const std::string_view &s)
{
    return to_cstr(s.data());
}

inline const char *to_cstr(const std::u8string_view &s)
{
    return to_cstr(s.data());
}

template<size_t StaticBufferCapacity> const char *to_cstr(const StaticBuffer<StaticBufferCapacity> &buf)
{
    return to_cstr((const char *)(buf.buf));
}

// cast char buf to char8_t buf
// this may break the strict-aliasing rule

inline const char8_t * to_u8cstr(const char8_t *s)
{
    if(s == nullptr){
        return u8"(null)";
    }
    else if(s[0] == '\0'){
        return u8"(empty)";
    }
    else{
        return s;
    }
}

inline const char8_t *to_u8cstr(const unsigned char *s)
{
    return reinterpret_cast<const char8_t *>(s);
}

inline const char8_t *to_u8cstr(const char *s)
{
    return reinterpret_cast<const char8_t *>(s);
}

inline const char8_t *to_u8cstr(const std::u8string &s)
{
    return to_u8cstr(s.c_str());
}

inline const char8_t *to_u8cstr(const std::string &s)
{
    return to_u8cstr(s.c_str());
}

template<size_t StaticBufferCapacity> const char8_t *to_u8cstr(const StaticBuffer<StaticBufferCapacity> &buf)
{
    return to_u8cstr((const char8_t *)(buf.buf));
}

inline const char *to_boolcstr(bool b)
{
    return b ? "true" : "false";
}

inline const char8_t *to_boolu8cstr(bool b)
{
    return b ? u8"true" : u8"false";
}

template<typename T> auto to_sv(const T &t)
{
    return std::string_view(to_cstr(t));
}

template<typename T> auto to_u8sv(const T &t)
{
    return std::u8string_view(to_u8cstr(t));
}

template<typename T> int to_boolint(T t)
{
    return (bool)(t) ? 1 : 0;
}

inline bool to_parsedbool(const char *s)
{
    if(!s){
        throw std::runtime_error("to_bool: null string");
    }

    if(s[0] == '\0'){
        throw std::runtime_error("to_bool: zero-length string");
    }

    if(false
            || to_sv(s) == "1"
            || to_sv(s) == "true"
            || to_sv(s) == "True"
            || to_sv(s) == "TRUE"){
        return true;
    }

    if(false
            || to_sv(s) == "0"
            || to_sv(s) == "false"
            || to_sv(s) == "False"
            || to_sv(s) == "FALSE"){
        return false;
    }

    throw std::runtime_error(std::string("to_bool: invalid boolean string: ") + s);
}

inline bool to_parsedbool(const std::string &s)
{
    return to_parsedbool(s.c_str());
}

inline bool to_parsedbool(const std::string_view &s)
{
    return to_parsedbool(s.data());
}

inline bool to_bool(const void *val)
{
    return val != nullptr;
}

inline bool to_bool(std::integral auto val)
{
    return val != 0;
}

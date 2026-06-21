#include <ctime>
#include <chrono>
#include <vector>
#include <cstring>
#include <stdexcept>
#include "strf.hpp"
#include "raiitimer.hpp"

std::string str_now(const char *format)
{
    const auto sys_now = std::chrono::system_clock::now();
    const auto in_time = std::chrono::system_clock::to_time_t(sys_now);

    struct tm now_time;

#if defined(__MINGW32__) || defined(__MINGW64__) || defined(_MSC_VER)
    const auto res_time = localtime_s(&now_time, &in_time) ? nullptr : &now_time;
#else
    const auto res_time = localtime_r(&in_time, &now_time);
#endif

    if(res_time){
        std::stringstream ss;
        ss << std::put_time(res_time, format ? format : "%Y-%m-%d %X");
        return ss.str();
    }

    throw std::runtime_error("localtime() failed");
}

std::string str_localtime()
{
    return std::to_string(hres_tstamp::localtime());
}

bool str_haschar(const std::string        &s) { return !s.empty(); }
bool str_haschar(const std::u8string      &s) { return !s.empty(); }
bool str_haschar(const std::string_view   &s) { return !s.empty(); }
bool str_haschar(const std::u8string_view &s) { return !s.empty(); }

#define _macro_str_vprintf_body_s(s, format, ap) do \
{ \
    if(!format){ \
        throw std::invalid_argument("str_vprintf(s, nullptr, ap)"); \
    } \
\
    constexpr size_t trySize = 64; \
    if(s.capacity() < trySize){ \
        s.resize(trySize, '\0'); \
    } \
\
    int needLen = -1; \
    { \
        va_list ap_cpy; \
        va_copy(ap_cpy, ap); \
\
        needLen = std::vsnprintf(reinterpret_cast<char *>(s.data()), s.size(), reinterpret_cast<const char *>(format), ap_cpy); \
        va_end(ap_cpy); \
\
        if(needLen < 0){ \
            throw std::runtime_error(((std::string("std::vsnprintf(\"") + reinterpret_cast<const char *>(format)) + "\") returns ") + std::to_string(needLen)); \
        } \
\
        if((size_t)(needLen) + 1 <= s.size()){ \
            s.resize(needLen, '\0'); \
            return s; \
        } \
    } \
\
    s.resize(needLen + 1, '\0'); \
    { \
        va_list ap_cpy; \
        va_copy(ap_cpy, ap); \
\
        const int newNeedLen = std::vsnprintf(reinterpret_cast<char *>(s.data()), s.size(), reinterpret_cast<const char *>(format), ap_cpy); \
        va_end(ap_cpy); \
\
        if(newNeedLen < 0){ \
            throw std::runtime_error(((std::string("std::vsnprintf(\"") + reinterpret_cast<const char *>(format)) + "\") returns ") + std::to_string(newNeedLen)); \
        } \
\
        if((size_t)(newNeedLen) + 1 != s.size()){ \
            throw std::runtime_error(((std::string("std::vsnprintf(\"") + reinterpret_cast<const char *>(format)) + "\") returns different value: ") + std::to_string(needLen) + " vs " + std::to_string(newNeedLen)); \
        } \
\
        s.resize(newNeedLen, '\0'); \
        return s; \
    } \
}while(0)

std::string &str_vprintf(std::string &s, const char *format, va_list ap)
{
    _macro_str_vprintf_body_s(s, format, ap);
}

std::u8string &str_vprintf(std::u8string &s, const char8_t *format, va_list ap)
{
    _macro_str_vprintf_body_s(s, format, ap);
}

#define _macro_str_vprintf_body(type, format, ap) do \
{ \
    if(!format){ \
        throw std::invalid_argument("str_vprintf(nullptr, ap)"); \
    } \
\
    va_list ap_cpy; \
    va_copy(ap_cpy, ap); \
\
    try{ \
        type s; \
        str_vprintf(s, format, ap_cpy); \
        va_end(ap_cpy); \
        return s; \
    } \
    catch(...){ \
        va_end(ap_cpy); \
        throw; \
    } \
}while(0)

std::string str_vprintf(const char *format, va_list ap)
{
    _macro_str_vprintf_body(std::string, format, ap);
}

std::u8string str_vprintf(const char8_t *format, va_list ap)
{
    _macro_str_vprintf_body(std::u8string, format, ap);
}

std::string str_printf(const char *format, ...)
{
    std::string s;
    str_format(format, s);
    return s;
}

std::u8string str_printf(const char8_t *format, ...)
{
    std::u8string s;
    str_format(format, s);
    return s;
}

std::string &str_printf(std::string &s, const char *format, ...)
{
    str_format(format, s);
    return s;
}

std::u8string &str_printf(std::u8string &s, const char8_t *format, ...)
{
    str_format(format, s);
    return s;
}

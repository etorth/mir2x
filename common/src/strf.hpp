// try to support printf to a std::string
// need to take care of va_list
//
// a). C++14 say nothing for va_list, it's from C
// b). C99/11 states that(C99-7.15, C11-7.16)
//    1. va_list can be re-initialized after called va_end()
//
//         void func_a(format, ...)
//         {
//             va_list ap;
//             va_start(format, ap);
//             ...
//             va_end(ap);
//
//             va_start(format, ap);
//             ...
//             va_end(ap);
//         }
//
//    2. va_list declared in func_a can be passed to func_b as an
//       argument, if func_b access va_list via va_arg then va_list
//       in func_a is indeterminate and shall be passed to va_end()
//       before any further reference
//
//         void func_b(va_list ap)
//         {
//             ...
//         }
//
//         void func_a(format, ...)
//         {
//             va_list ap;
//             va_start(format, ap);
//
//             func_b(ap)
//
//             // now ap is indeterminate
//             // no reference to ap before va_end()
//             // means func_a() can't see any change of ap in func_b()
//
//             va_end(ap);
//         }
//
//    3. va_list declared in func_a can be passed to func_b via a pointer
//       to it, in which case the original function may make further use
//       of the original va_list after func_b returns
//
//         void func_b(va_list *ap)
//         {
//             // pull out one argument from ap
//             // and handle it
//         }
//
//         void func_a(format, ...)
//         {
//             va_list ap;
//             va_start(format, ap);
//
//             func_b(&ap)
//
//             // now ap is still ok
//             // func_a() can see changes of ap in func_b()
//
//             func_b(&ap)
//             ...
//
//             va_end(ap);
//         }
//
// if we want to use this va_list in C++ safely, make a .a with a C99/11
// compiler and and call it from C++

#pragma once
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <vector>
#include <deque>
#include <string>
#include <cstdarg>
#include <concepts>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

#ifdef __GNUC__
    #define STR_PRINTF_CHECK_FORMAT(n) __attribute__ ((format (printf, (n), ((n)+1))))
#else
    #define STR_PRINTF_CHECK_FORMAT(n)
#endif

constexpr bool str_haschar(const char *s)
{
    return s && s[0] != '\0';
}

constexpr bool str_haschar(const char8_t *s)
{
    return s && s[0] != '\0';
}

bool str_haschar(const std::string &);
bool str_haschar(const std::u8string &);

bool str_haschar(const std::string_view &);
bool str_haschar(const std::u8string_view &);

template<typename T> str_quoted(const T &s)
{
    std::stringstream ss;
    ss << std::quoted(s);
    return ss.str();
}

template<std::integral T> [[nodiscard]] std::string str_ksep(T t, char sep = ',')
{
    std::string result;
    std::string numstr = std::to_string(t);

    bool neg = false;
    std::reverse(numstr.begin(), numstr.end());

    if(numstr.back() == '-'){
        numstr.pop_back();
        neg = true;
    }

    for(size_t i = 0; i < numstr.size(); ++i){
        result.push_back(numstr[i]);
        if(i % 3 == 2){
            result.push_back(sep);
        }
    }

    if(result.back() == sep){
        result.pop_back();
    }

    if(neg){
        result.push_back('-');
    }

    std::reverse(result.begin(), result.end());
    return result;
}

[[nodiscard]] std::string str_printf(const char *, ...) STR_PRINTF_CHECK_FORMAT(1);
[[nodiscard]] std::string str_vprintf(const char *, va_list);

[[maybe_unused]] std::string &str_printf(std::string &, const char *, ...) STR_PRINTF_CHECK_FORMAT(2);
[[maybe_unused]] std::string &str_vprintf(std::string &, const char *, va_list);

[[nodiscard]] std::u8string str_printf(const char8_t *, ...);
[[nodiscard]] std::u8string str_vprintf(const char8_t *, va_list);

[[maybe_unused]] std::u8string &str_printf(std::u8string &, const char8_t *, ...);
[[maybe_unused]] std::u8string &str_vprintf(std::u8string &, const char8_t *, va_list);

#define str_format(format, s) do\
{\
    if(!format){\
        throw std::invalid_argument("str_format(nullptr, ap)"); \
    }\
    va_list ap;\
    va_start(ap, format);\
    try{\
        str_vprintf(s, format, ap);\
        va_end(ap);\
    }\
    catch(...){\
        va_end(ap);\
        throw std::runtime_error(str_printf("call str_vprintf(%s) failed", (const char *)(format)));\
    }\
}while(0)\

inline std::string str_any(const char *s)
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

inline std::string str_any(const char8_t *s)
{
    return str_any((const char *)(s));
}

inline std::string str_any(const std::string &s)
{
    return str_any(s.c_str());
}

inline std::string str_any(const std::u8string &s)
{
    return str_any((const char *)(s.c_str()));
}

inline std::string str_any(const std::string_view &s)
{
    return str_any(std::string(s.data(), s.size()));
}

inline std::string str_any(const std::u8string_view &s)
{
    return str_any(std::u8string(s.data(), s.size()));
}

inline std::string str_any(char ch)
{
    if(ch == '\0'){
        return "\'\\0\'";
    }
    else{
        return str_printf("\'%c\'", ch);
    }
}

inline std::string str_any(bool b)
{
    return b ? "true" : "false";
}

template<typename T> std::string str_any(const std::set<T> &);
template<typename T> std::string str_any(const std::unordered_set<T> &);

template<typename K, typename V> std::string str_any(const std::map<K, V> &);
template<typename K, typename V> std::string str_any(const std::unordered_map<K, V> &);

template<typename T> std::string str_any(const std::list<T> &);
template<typename T> std::string str_any(const std::deque<T> &);
template<typename T> std::string str_any(const std::vector<T> &);
template<typename U, typename V> std::string str_any(const std::pair<U, V> &);

template<typename T> std::string str_any(const T &t)
{
    std::stringstream ss;
    return dynamic_cast<std::stringstream &>(ss << t).str();
}

#define _str_any_container_helper(c) \
{ \
    std::string result = "{"; \
    for(const auto &item_in_c: c){ \
        if(result.size() > 1){ \
            result += ","; \
        } \
        result += str_any(item_in_c); \
    } \
    return result + "}"; \
}

template<typename T> std::string str_any(const std::set<T> &s)           _str_any_container_helper(s)
template<typename T> std::string str_any(const std::unordered_set<T> &s) _str_any_container_helper(s)

template<typename K, typename V> std::string str_any(const std::map<K, V> &m)           _str_any_container_helper(m)
template<typename K, typename V> std::string str_any(const std::unordered_map<K, V> &m) _str_any_container_helper(m)

template<typename T> std::string str_any(const std::list<T> &l)   _str_any_container_helper(l)
template<typename T> std::string str_any(const std::deque<T> &q)  _str_any_container_helper(q)
template<typename T> std::string str_any(const std::vector<T> &v) _str_any_container_helper(v)
#undef _str_any_container_helper

template<typename K, typename V> std::string str_any(const std::pair<K, V> &p)
{
    return std::string("{") + str_any(p.first) + "," + str_any(p.second) + "}";
}

template<std::size_t I, typename... T> inline typename std::enable_if<I == sizeof...(T), std::string>::type _str_any_tuple_helper(const std::tuple<T...> &)
{
    return {};
}

template<std::size_t I, typename... T> inline typename std::enable_if<I <  sizeof...(T), std::string>::type _str_any_tuple_helper(const std::tuple<T...> &t)
{
    return std::string((I == 0) ? "" : ",") + str_any(std::get<I>(t)) + _str_any_tuple_helper<I + 1>(t);
}

template<typename... T> std::string str_any(const std::tuple<T...> &t)
{
    return "{" + _str_any_tuple_helper<0>(t) + "}";
}

// copy from boost
// definition of BOOST_CURRENT_FUNCTION

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600))
    #define str_ffl() str_printf("In file: %s:%d, function: %s", std::filesystem::path(__FILE__).filename().c_str(), __LINE__, __PRETTY_FUNCTION__)
#elif defined(__DMC__) && (__DMC__ >= 0x810)
    #define str_ffl() str_printf("In file: %s:%d, function: %s", std::filesystem::path(__FILE__).filename().c_str(), __LINE__, __PRETTY_FUNCTION__)
#elif defined(__FUNCSIG__)
    #define str_ffl() str_printf("In file: %s:%d, function: %s", std::filesystem::path(__FILE__).filename().c_str(), __LINE__,         __FUNCSIG__)
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
    #define str_ffl() str_printf("In file: %s:%d, function: %s", std::filesystem::path(__FILE__).filename().c_str(), __LINE__,        __FUNCTION__)
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    #define str_ffl() str_printf("In file: %s:%d, function: %s", std::filesystem::path(__FILE__).filename().c_str(), __LINE__,            __FUNC__)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
    #define str_ffl() str_printf("In file: %s:%d, function: %s", std::filesystem::path(__FILE__).filename().c_str(), __LINE__,            __func__)
#else
    #define str_ffl() str_printf("In file: %s:%d, function: %s", std::filesystem::path(__FILE__).filename().c_str(), __LINE__,         "(unknown)")
#endif

/*
 * =====================================================================================
 *
 *       Filename: strf.hpp
 *        Created: 11/27/2018 22:28:55
 *    Description: 
 *                  try to support printf to a std::string
 *                  need to take care of va_list
 *
 *                  a). C++14 say nothing for va_list, it's from C
 *                  b). C99/11 states that(C99-7.15, C11-7.16)
 *                     1. va_list can be re-initialized after called va_end()
 *
 *                          void func_a(format, ...)
 *                          {
 *                              va_list ap;
 *                              va_start(format, ap);
 *                              ...
 *                              va_end(ap);
 *
 *                              va_start(format, ap);
 *                              ...
 *                              va_end(ap);
 *                          }
 *
 *                     2. va_list declared in func_a can be passed to func_b as an
 *                        argument, if func_b access va_list via va_arg then va_list
 *                        in func_a is indeterminate and shall be passed to va_end()
 *                        before any further reference
 *
 *                          void func_b(va_list ap)
 *                          {
 *                              ...
 *                          }
 *
 *                          void func_a(format, ...)
 *                          {
 *                              va_list ap;
 *                              va_start(format, ap);
 *
 *                              func_b(ap)
 *
 *                              // now ap is indeterminate
 *                              // no reference to ap before va_end()
 *                              // means func_a() can't see any change of ap in func_b()
 *
 *                              va_end(ap);
 *                          }
 *
 *                     3. va_list declared in func_a can be passed to func_b via a pointer
 *                        to it, in which case the original function may make further use
 *                        of the original va_list after func_b returns
 *
 *                          void func_b(va_list *ap)
 *                          {
 *                              // pull out one argument from ap
 *                              // and handle it
 *                          }
 *
 *                          void func_a(format, ...)
 *                          {
 *                              va_list ap;
 *                              va_start(format, ap);
 *
 *                              func_b(&ap)
 *
 *                              // now ap is still ok
 *                              // func_a() can see changes of ap in func_b()
 *
 *                              func_b(&ap)
 *                              ...
 *
 *                              va_end(ap);
 *                          }
 *
 *                  if we want to use this va_list in C++ safely, make a .a with a C99/11
 *                  compiler and and call it from C++
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <string>
#include <cstdarg>
#include <concepts>
#include <algorithm>
#include <filesystem>

#ifdef __GNUC__
    #define STR_PRINTF_CHECK_FORMAT(n) __attribute__ ((format (printf, (n), ((n)+1))))
#else
    #define STR_PRINTF_CHECK_FORMAT(n)
#endif

bool str_haschar(const char *);
bool str_haschar(const char8_t *);

bool str_haschar(const std::string &);
bool str_haschar(const std::u8string &);

bool str_haschar(const std::string_view &);
bool str_haschar(const std::u8string_view &);

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

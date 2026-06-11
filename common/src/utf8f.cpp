#include <utf8.h>
#include <cstring>
#include <cstdint>
#include <iterator>
#include "strf.hpp"
#include "utf8f.hpp"
#include "fflerror.hpp"

namespace
{
    bool validCodePoint(uint32_t codePoint)
    {
        return codePoint <= 0X10FFFF && !(codePoint >= 0XD800 && codePoint <= 0XDFFF);
    }

    uint32_t str2codeImpl(const char *utf8Begin, size_t utf8Size, size_t &pos)
    {
        if(pos >= utf8Size){
            throw fflpanic("no UTF-8 character at position: {}", pos);
        }

        if(!utf8Begin){
            throw fflpanic("null UTF-8 string");
        }

        auto p = utf8Begin + pos;
        const auto p0 = p;
        const auto pend = utf8Begin + utf8Size;

        uint32_t codePoint = 0;
        try{
            codePoint = utf8::next(p, pend);
        }
        catch(...){
            throw fflpanic("failed to convert UTF-8 string to code point");
        }

        const auto charLen = p - p0;
        if(charLen <= 0 || charLen > 4){
            throw fflpanic("invalid UTF-8 character length: {}", charLen);
        }

        if(!validCodePoint(codePoint)){
            throw fflpanic("invalid UTF-8 code point: 0x{:X}", codePoint);
        }

        pos += static_cast<size_t>(charLen);
        return codePoint;
    }

    std::string peekFirstImpl(std::string_view utf8String)
    {
        size_t pos = 0;
        str2codeImpl(utf8String.data(), utf8String.size(), pos);
        return std::string(utf8String.data(), pos);
    }

    std::string peekLastImpl(std::string_view utf8String)
    {
        if(utf8String.empty()){
            throw fflpanic("no UTF-8 character in empty string");
        }

        if(!utf8String.data()){
            throw fflpanic("null UTF-8 string");
        }

        size_t pos = utf8String.size() - 1;
        while(pos > 0){
            const auto ch = static_cast<unsigned char>(utf8String[pos]);
            if((ch & 0XC0) != 0X80){
                break;
            }
            pos--;
        }

        if(utf8String.size() - pos > 4){
            throw fflpanic("invalid UTF-8 character length: {}", utf8String.size() - pos);
        }

        auto endPos = pos;
        str2codeImpl(utf8String.data(), utf8String.size(), endPos);
        if(endPos != utf8String.size()){
            throw fflpanic("invalid trailing UTF-8 character");
        }

        return std::string(utf8String.data() + pos, utf8String.size() - pos);
    }
}

std::string utf8f::code2str(uint32_t codePoint)
{
    if(!validCodePoint(codePoint)){
        throw fflpanic("invalid UTF-8 code point: 0x{:X}", codePoint);
    }

    std::string result;
    try{
        utf8::append(codePoint, std::back_inserter(result));
    }
    catch(...){
        throw fflpanic("failed to convert code point to UTF-8 string: 0x{:X}", codePoint);
    }

    if(result.empty() || result.size() > 4){
        throw fflpanic("invalid UTF-8 string converted from code point: 0x{:X}", codePoint);
    }
    return result;
}

uint32_t utf8f::str2code(const char *utf8String)
{
    size_t pos = 0;
    return str2code(utf8String, pos);
}

uint32_t utf8f::str2code(const char *utf8String, size_t &pos)
{
    if(!utf8String){
        throw fflpanic("null UTF-8 string");
    }
    return str2codeImpl(utf8String, std::strlen(utf8String), pos);
}

uint32_t utf8f::str2code(const std::string utf8String)
{
    return str2code(std::string_view(utf8String));
}

uint32_t utf8f::str2code(const std::string utf8String, size_t &pos)
{
    return str2code(std::string_view(utf8String), pos);
}

uint32_t utf8f::str2code(const std::string_view &utf8String)
{
    size_t pos = 0;
    return str2code(utf8String, pos);
}

uint32_t utf8f::str2code(const std::string_view &utf8String, size_t &pos)
{
    return str2codeImpl(utf8String.data(), utf8String.size(), pos);
}

std::string utf8f::peekFirst(const char *utf8String)
{
    if(!utf8String){
        throw fflpanic("null UTF-8 string");
    }
    return peekFirst(std::string_view(utf8String));
}

std::string utf8f::peekFirst(const std::string &utf8String)
{
    return peekFirst(std::string_view(utf8String));
}

std::string utf8f::peekFirst(const std::string_view &utf8String)
{
    return peekFirstImpl(utf8String);
}

std::string utf8f::peekLast(const char *utf8String)
{
    if(!utf8String){
        throw fflpanic("null UTF-8 string");
    }
    return peekLast(std::string_view(utf8String));
}

std::string utf8f::peekLast(const std::string &utf8String)
{
    return peekLast(std::string_view(utf8String));
}

std::string utf8f::peekLast(const std::string_view &utf8String)
{
    return peekLastImpl(utf8String);
}

std::vector<int> utf8f::buildUTF8Off(const char *utf8String)
{
    fflassert(utf8String);
    const auto size = std::strlen(utf8String);

    if(size == 0){
        return {};
    }

    std::vector<int> off;

    const char *p = utf8String;
    const char *pend = utf8String + size;

    off.reserve(size);
    for(; p < pend; utf8::advance(p, 1, pend)){
        off.push_back(p - utf8String);
    }

    return off;
}

std::string utf8f::toupper(std::string s)
{
    char *p = s.data();
    char *pend = s.data() + s.size();

    while(p < pend){
        auto lastp = p;
        utf8::advance(p, 1, pend);

        if(p - lastp == 1 && *lastp >= 'a' && *lastp <= 'z'){
            *lastp = 'A' + *lastp - 'a';
        }
    }
    return s;
}

bool utf8f::valid(const std::string &s)
{
    return utf8::is_valid(s);
}

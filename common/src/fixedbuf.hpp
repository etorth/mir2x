#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <cstring>
#include <type_traits>
#include "cerealf.hpp"
#include "fflerror.hpp"

// we transfer chunks of data/string too much
// when we don't use cerealf, we need a handy static buffer class in messages

template<size_t FixedBufSize> struct FixedBuf
{
    uint8_t buf[FixedBufSize + 1]; // reserve tail '\0' for strings
    uint32_t size;

    bool empty() const
    {
        return size == 0;
    }

    void clear()
    {
        size = 0;
    }

    constexpr size_t capacity() const
    {
        return FixedBufSize;
    }

    void assign(const void *data, size_t length)
    {
        if(length > 0){
            fflassert(data);
            fflassert(length <= capacity());
            std::memcpy(buf, data, length);
        }
        size = length;
    }

    void assign(std::string_view s)
    {
        assign(s.data(), s.size());
        buf[size] = 0;
    }

    void assign(std::u8string_view s)
    {
        assign(s.data(), s.size());
        buf[size] = 0;
    }

    void assign(const std::string &s)
    {
        assign(std::string_view(s.data(), s.size()));
    }

    void assign(const std::u8string &s)
    {
        assign(std::u8string_view(s.data(), s.size()));
    }

    void assign(const char *s)
    {
        assign(std::string_view(s));
    }

    void assign(const char8_t *s)
    {
        assign(std::u8string_view(s));
    }

    template<typename T> void assign(const T &t)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        assign(&t, sizeof(t));
    }

    template<typename T> T as() const
    {
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(sizeof(T) == size);

        T t;
        std::memcpy(&t, buf, sizeof(T));
        return t;
    }

    std::string_view as_sv() const
    {
        return std::string_view((const char *)(buf), size);
    }

    std::u8string_view as_u8sv() const
    {
        return std::u8string_view((const char8_t *)(buf), size);
    }

    std::string to_str() const
    {
        return std::string((const char *)(buf), size);
    }

    std::u8string to_u8str() const
    {
        return std::u8string((const char8_t *)(buf), size);
    }

    template<typename T> void serialize(const T &t, int tryComp = -1)
    {
        assign(cerealf::serialize<T>(t, tryComp));
    }

    template<typename T> T deserialize() const
    {
        return cerealf::deserialize<T>(buf, size);
    }
};
static_assert(std::is_trivially_copyable_v<FixedBuf<128>>);

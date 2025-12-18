#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <cstring>
#include <type_traits>
#include "cerealf.hpp"
#include "fflerror.hpp"
#include "conceptf.hpp"

// we transfer chunks of data/string too much
// when we don't use cerealf, we need a handy static buffer class in messages

template<size_t Capacity> struct StaticBuffer
{
    uint16_t size;
    uint8_t data[Capacity + 1]; // reserve tail '\0' for strings

    bool empty() const
    {
        return size == 0;
    }

    void clear()
    {
        size = 0;
    }

    constexpr static size_t capacity()
    {
        return Capacity;
    }

    void assign(const void *data, size_t length)
    {
        if(length > 0){
            if(!data){
                throw fflerror("null data pointer while data length is non-zero: %zu", length);
            }

            if(length > capacity()){
                throw fflerror("data length %zu exceeds buffer capacity: %zu", length, capacity());
            }

            std::memcpy(this->data, data, length);
        }
        size = length;
    }

    void assign(std::string_view s)
    {
        assign(s.data(), s.size());
        data[size] = 0;
    }

    void assign(std::u8string_view s)
    {
        assign(s.data(), s.size());
        data[size] = 0;
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
        if constexpr (std::is_trivially_copyable_v<T>){
            assign(&t, sizeof(t));
        }
        else{
            serialize(t);
        }
    }

    template<conceptf::TriviallyCopyable T> T as() const
    {
        T t;
        std::memcpy(&t, this->data, sizeof(T));
        return t;
    }

    const char *as_rawcstr() const
    {
        return (const char *)(this->data);
    }

    const char8_t *as_rawu8cstr() const
    {
        return (const char8_t *)(this->data);
    }

    const char *as_cstr() const
    {
        return empty() ? "(empty)" : as_rawcstr();
    }

    const char8_t *as_u8str() const
    {
        return empty() ? u8"(empty)" : as_rawu8cstr();
    }

    std::string_view as_sv() const
    {
        return std::string_view((const char *)(this->data), size);
    }

    std::u8string_view as_u8sv() const
    {
        return std::u8string_view((const char8_t *)(this->data), size);
    }

    std::string to_str() const
    {
        return std::string((const char *)(this->data), size);
    }

    std::u8string to_u8str() const
    {
        return std::u8string((const char8_t *)(this->data), size);
    }

    template<typename T> void serialize(const T &t, int tryComp = -1)
    {
        assign(cerealf::serialize<T>(t, tryComp));
    }

    template<typename T> T deserialize() const
    {
        return cerealf::deserialize<T>(this->data, size);
    }
};
static_assert(std::is_trivially_copyable_v<StaticBuffer<128>>);

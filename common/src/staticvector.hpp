#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <cstring>
#include <type_traits>
#include "conceptf.hpp"
#include "fflerror.hpp"

template<conceptf::TriviallyCopyable T, size_t Capacity> struct StaticVector
{
    uint16_t size;
    T data[Capacity];

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

    /**/  T &front()       { if(empty()){ throw fflpanic("empty vector"); } return data[0]; }
    const T &front() const { if(empty()){ throw fflpanic("empty vector"); } return data[0]; }
    /**/  T & back()       { if(empty()){ throw fflpanic("empty vector"); } return data[size - 1]; }
    const T & back() const { if(empty()){ throw fflpanic("empty vector"); } return data[size - 1]; }

    /**/  T *begin()       noexcept { return data; }
    const T *begin() const noexcept { return data; }
    /**/  T *  end()       noexcept { return data + size; }
    const T *  end() const noexcept { return data + size; }

    void push(const T &t)
    {
        if(size >= capacity()){
            throw fflpanic("capacity exceeds");
        }
        else{
            data[size++] = t;
        }
    }

    void pop()
    {
        if(size > 0){
            size--;
        }
        else{
            throw fflpanic("emtpy vector");
        }
    }

    template<class InputIt> void assign(InputIt first, InputIt last)
    {
        if(const auto inputsize = std::distance(first, last); inputsize < 0){
            throw fflpanic("invalid range");
        }

        else if((size_t)(inputsize) > capacity()){
            throw fflpanic("size of range {} exceeds capacity {}", (size_t)(inputsize), capacity());
        }
        else{
            size = inputsize;
            std::copy(first, last, data);
        }
    }

    std::span<T> as_span()
    {
        return std::span<T>(data, size);
    }

    std::span<const T> as_span() const
    {
        return std::span<T>(data, size);
    }
};

static_assert(std::is_trivially_copyable_v<StaticVector<char, 128>>);

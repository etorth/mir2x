#pragma once
#include <cstdio>
#include <memory>
#include <cerrno>
#include <cstring>
#include <cstdint>
#include <climits>
#include <concepts>
#include <type_traits>
#include "strf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

namespace _fileptr_details
{
    template<typename C> size_t size_in_bytes(size_t size)
    {
        if(constexpr auto bytes = sizeof(typename C::value_type); size > SIZE_MAX / bytes){
            throw fflpanic("size overflows: {} * {}", size, bytes);
        }
        else{
            return size * bytes;
        }
    }
}

inline auto make_fileptr_helper(const char *path, const char *mode)
{
    fflassert(str_haschar(path), path);
    fflassert(str_haschar(mode), mode);

    errno = 0;
    if(auto fp = std::fopen(path, mode); fp){
        constexpr auto fileptr_deleter = [](std::FILE *fp)-> void
        {
            // don't need to check fp
            // deleter only get called when fp is not null
            errno = 0;
            if(std::fclose(fp)){
                const auto err = errno;
                std::fprintf(stderr, "fclose() failed: [%d] %s\n", err, std::strerror(err));
            }
        };

        // use a wrapper to the std::fclose
        // avoid pass standard lib's function pointer
        return std::unique_ptr<std::FILE, decltype(fileptr_deleter)>(fp, fileptr_deleter);
    }

    const auto err = errno;
    throw fflpanic("fopen({}, {}) failed: [{}] {}", str_quoted(path), str_quoted(mode), err, std::strerror(err));
}

inline auto make_fileptr(const char *path, const char *mode)
{
    return make_fileptr_helper(path, mode);
}

inline auto make_fileptr(const char8_t *path, const char *mode)
{
    return make_fileptr_helper(reinterpret_cast<const char *>(path), mode);
}

// define a standalone type of fileptr_t
// but can't use it to instantiate an uninitalized object:
//
// fileptr_t p_null;                                // wrong: std::unique<std::FILE, deleter> doesn't have default constructor
// fileptr_t p_good = make_fileptr("123.txt", "r"); // good!
//
//
// struct test
// {
//     fileptr_t m_good_ptr;
//     fileptr_t m_null_ptr;
//     test()
//       : m_good_ptr(make_fileptr("123.txt", "r"))    // good
//       , m_null_ptr(nullptr)                         // wrong
//       , m_null_ptr()                                // wrong
//     {}
// }
using fileptr_t = std::invoke_result_t<decltype(&make_fileptr_helper), const char *, const char *>;

inline size_t tell_fileptr(fileptr_t &fptr)
{
    fflassert(fptr);
    errno = 0;

    if(const auto loc = ftello(fptr.get()); loc >= 0){
        return check_cast<size_t>(loc);
    }

    const auto err = errno;
    throw fflpanic("ftello({:p}) failed: [{}] {}", to_cvptr(fptr.get()), err, std::strerror(err));
}

template<std::signed_integral T> void seek_fileptr(fileptr_t &fptr, T offset, int origin)
{
    fflassert(fptr);
    errno = 0;

    if(fseeko(fptr.get(), offset, origin)){
        const auto err = errno;
        throw fflpanic("fseeko({:p}, {}, {}) failed: [{}] {}", to_cvptr(fptr.get()), offset, [origin]() -> const char *
        {
            switch(origin){
                case SEEK_SET: return "SEEK_SET";
                case SEEK_CUR: return "SEEK_CUR";
                case SEEK_END: return "SEEK_END";
                default      : return "SEEK_???";
            }
        }(), err, std::strerror(err));
    }
}

inline size_t size_fileptr(fileptr_t &fptr, size_t *curLoc = nullptr)
{
    const auto oldLoc = tell_fileptr(fptr);
    seek_fileptr(fptr, 0, SEEK_END);

    const auto endLoc = tell_fileptr(fptr);
    seek_fileptr(fptr, check_cast<int64_t>(oldLoc), SEEK_SET);

    if(curLoc){
        *curLoc = oldLoc;
    }
    return endLoc;
}

inline void read_fileptr(fileptr_t &fptr, void *data, size_t size)
{
    fflassert(fptr);
    if(size > 0){
        fflassert(data);
        errno = 0;

        // read starting from current position
        // read exact size bytes

        if(const auto reads = std::fread(data, 1, size, fptr.get()); reads != size){
            const auto err = errno;
            throw fflpanic("fread({:p}, 1, {}, {:p}) returns {}: [{}] {}", to_cvptr(data), size, to_cvptr(fptr.get()), reads, err, std::strerror(err));
        }
    }
}

template<typename C> void read_fileptr(fileptr_t &fptr, C &c, size_t size)
{
    fflassert(fptr);

    c.resize(size);
    read_fileptr(fptr, c.data(), _fileptr_details::size_in_bytes<C>(size));
}

template<typename C> C read_fileptr(fileptr_t &fptr, size_t size)
{
    fflassert(fptr);

    C c;
    read_fileptr<C>(fptr, c, size);
    return c;
}

template<typename C> C read_fileptr(fileptr_t &fptr)
{
    fflassert(fptr);

    size_t pos = 0;
    const auto size = size_fileptr(fptr, &pos);

    fflassert(size >= pos);

    const auto quot = (size - pos) / sizeof(typename C::value_type);
    const auto rem  = (size - pos) % sizeof(typename C::value_type);

    fflassert(rem == 0);

    C c;
    read_fileptr<C>(fptr, c, quot);
    return c;
}

inline void write_fileptr(fileptr_t &fptr, const void *data, size_t size)
{
    fflassert(fptr);
    if(size > 0){
        fflassert(data);
        errno = 0;

        if(const auto writes = std::fwrite(data, 1, size, fptr.get()); writes != size){
            const auto err = errno;
            throw fflpanic("fwrite({:p}, 1, {}, {:p}) returns {}: [{}] {}", to_cvptr(data), size, to_cvptr(fptr.get()), writes, err, std::strerror(err));
        }
    }
}

template<typename C> void write_fileptr(fileptr_t &fptr, const C &c)
{
    fflassert(fptr);
    if constexpr(std::is_trivially_copyable_v<C>){
        write_fileptr(fptr, &c, sizeof(c));
    }
    else{
        write_fileptr(fptr, c.data(), _fileptr_details::size_in_bytes<C>(c.size()));
    }
}

inline void flush_fileptr(fileptr_t &fptr)
{
    fflassert(fptr);
    errno = 0;

    if(std::fflush(fptr.get())){
        const auto err = errno;
        throw fflpanic("fflush({:p}) failed: [{}] {}", to_cvptr(fptr.get()), err, std::strerror(err));
    }
}

inline void close_fileptr(fileptr_t &fptr)
{
    if(fptr){
        errno = 0;
        if(std::fclose(fptr.release())){
            const auto err = errno;
            throw fflpanic("fclose() failed: [{}] {}", err, std::strerror(err));
        }
    }
}

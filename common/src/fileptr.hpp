#pragma once
#include <cstdio>
#include <memory>
#include <cerrno>
#include <cstring>
#include <type_traits>
#include "strf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

inline auto make_fileptr_helper(const char *path, const char *mode)
{
    if(auto fp = std::fopen(path, mode); fp){
        constexpr auto fileptr_deleter = [](std::FILE *fp)-> void
        {
            // don't need to check fp
            // deleter only get called when fp is not null
            if(std::fclose(fp)){
                // we shouldn't throw
                // how to handle the failure here ?
            }
        };

        // use a wrapper to the std::fclose
        // avoid pass standard lib's function pointer
        return std::unique_ptr<std::FILE, decltype(fileptr_deleter)>(fp, fileptr_deleter);
    }
    throw fflerror("failed to open file: [%p] \"%s\", mode: [%p] \"%s\", errno: [%d] \"%s\"", to_cvptr(path), to_cstr(path), to_cvptr(mode), to_cstr(mode), to_d(errno), std::strerror(errno));
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

// file stream ops
// wrapper to std::seek/std::fread/std::fwrite
inline size_t tell_fileptr(fileptr_t &fptr)
{
    if(const auto loc = std::ftell(fptr.get()); loc >= 0){
        return to_uz(loc);
    }
    throw fflerror("failed to ftell file: err = %s", std::strerror(errno));
}

inline void seek_fileptr(fileptr_t &fptr, size_t offset, int origin)
{
    if(std::fseek(fptr.get(), check_cast<long>(offset), origin)){
        throw fflerror("failed to seek file: offset = %zu, origin = %s, err = %s", offset, [origin]() -> const char *
        {
            switch(origin){
                case SEEK_SET: return "SEEK_SET";
                case SEEK_CUR: return "SEEK_CUR";
                case SEEK_END: return "SEEK_END";
                default      : return "SEEK_???";
            }
        }(), std::strerror(errno));
    }
}

inline size_t size_fileptr(fileptr_t &fptr)
{
    const auto oldLoc = tell_fileptr(fptr);
    seek_fileptr(fptr, 0, SEEK_END);

    const auto endLoc = tell_fileptr(fptr);
    seek_fileptr(fptr, oldLoc, SEEK_SET);
    return endLoc;
}

inline void read_fileptr(fileptr_t &fptr, void *data, size_t size)
{
    fflassert(fptr);
    fflassert(data);
    fflassert(size > 0);

    if(std::fread(data, size, 1, fptr.get()) != 1){
        throw fflerror("failed to read file: data = %p, size = %zu, err = %s", to_cvptr(data), size, std::strerror(errno));
    }
}

template<typename C> void read_fileptr(fileptr_t &fptr, C &c, size_t size)
{
    fflassert(fptr);
    fflassert(size > 0);

    c.resize(size);
    read_fileptr(fptr, c.data(), size * sizeof(typename C::value_type));
}

template<typename C> C read_fileptr(fileptr_t &fptr, size_t size)
{
    fflassert(fptr);
    fflassert(size > 0);

    C c;
    read_fileptr<C>(fptr, c, size);
    return c;
}

template<typename C> C read_fileptr(fileptr_t &fptr)
{
    fflassert(fptr);
    const auto size = size_fileptr(fptr);

    if(size % sizeof(typename C::value_type)){
        throw fflerror("file size alignment error: file size %zu, element size %zu", size, sizeof(C::value_type));
    }

    C c;
    read_fileptr<C>(fptr, c, size / sizeof(typename C::value_type));
    return c;
}

inline void write_fileptr(fileptr_t &fptr, const void *data, size_t size)
{
    fflassert(fptr);
    fflassert(data);
    fflassert(size > 0);

    if(std::fwrite(data, size, 1, fptr.get()) != 1){
        throw fflerror("failed to write file: data = %p, size = %zu, err = %s", to_cvptr(data), size, std::strerror(errno));
    }
}

template<typename C> void write_fileptr(fileptr_t &fptr, const C &c)
{
    fflassert(fptr);
    if constexpr(std::is_trivially_copyable_v<C>){
        write_fileptr(fptr, &c, sizeof(c));
    }
    else{
        write_fileptr(fptr, c.data(), c.size() * sizeof(typename C::value_type));
    }
}

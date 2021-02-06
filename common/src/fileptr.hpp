/*
 * =====================================================================================
 *
 *       Filename: fileptr.hpp
 *        Created: 11/29/2018 08:25:27
 *    Description: give a little automation to file operations
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
    throw fflerror("failed to open file: [%p]%s, mode: [%p]%s: %s", to_cvptr(path), to_cstr(path), to_cvptr(mode), to_cstr(mode), std::strerror(errno));
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

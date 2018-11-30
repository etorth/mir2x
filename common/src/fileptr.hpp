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

#include <cstdio>
#include <memory>
#include <cerrno>
#include <cstring>
#include "strfunc.hpp"

inline auto make_fileptr(const char *path, const char *mode)
{
    if(auto fp = std::fopen(path, mode); fp){
        auto file_closer = [](std::FILE *fp) -> void
        {
            // don't need to check fp
            // deleter only get called when fp is not null
            if(std::fclose(fp)){
                // we shouldn't throw
                // how to handle the failure...
            }
        };

        // use a wrapper to the file_closer
        // avoid pass standard lib's function pointer std::fclose
        return std::unique_ptr<std::FILE, decltype(file_closer)>(fp, file_closer);
    }
    throw std::runtime_error(str_printf("in make_fileptr(%s, %s): %s", path ? path : "(null)", mode ? mode : "(null)", std::strerror(errno)));
}

// seems I can't support this
// using fileptr = std::invoke_result_t<decltype(&make_fileptr), const char *, const char *>;

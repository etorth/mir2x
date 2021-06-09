/*
 * =====================================================================================
 *
 *       Filename: pngf.hpp
 *        Created: 02/06/2016 04:25:06
 *    Description: 
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
#include <cstdint>
#include "totype.hpp"
#include "fflerror.hpp"

namespace pngf
{
    bool saveRGBABuffer(const void *, uint32_t, uint32_t, const char *);
    inline void saveRGBABufferEx(const void *data, uint32_t width, uint32_t height, const char *fileName)
    {
        if(!saveRGBABuffer(data, width, height, fileName)){
            throw fflerror("saveRGBABuffer(%p, %llu, %llu, [%p]:%s) failed", data, to_llu(width), to_llu(height), fileName, fileName ? fileName : "(null)");
        }
    }
}

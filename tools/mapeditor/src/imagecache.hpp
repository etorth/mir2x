/*
 * =====================================================================================
 *
 *       Filename: imagecache.hpp
 *        Created: 02/14/2016 15:40:17
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
#include <vector>
#include <memory>
#include <unordered_map>
#include <FL/Fl_RGB_Image.H>
#include "totype.hpp"
#include "fflerror.hpp"

class ImageCache
{
    private:
        std::vector<uint32_t> m_convBuf;
        std::unordered_map<uint32_t, std::unique_ptr<Fl_Image>> m_cache;

    public:
        ImageCache() = default;

    public:
        Fl_Image *retrieve(uint32_t);

    public:
        Fl_Image *retrieve(uint8_t fileIndex, uint16_t imageIndex)
        {
            return retrieve((to_u32(fileIndex) << 16) + imageIndex);
        }

    public:
        void clear()
        {
            m_cache.clear();
        }
};

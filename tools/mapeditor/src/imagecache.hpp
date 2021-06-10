/*
 * =====================================================================================
 *
 *       Filename: imagecache.hpp
 *        Created: 02/14/2016 15:40:17
 *    Description: Interaction with WilImagePackage
 *                 return drawable objects with ImageKey provided by EditorMap
 *
 *                 if ImageCache::Retrieve() is not nullptr, then this class guartanee
 *                 there must be a corresponding .PNG file in the cache folder
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
#include <memory>
#include <unordered_map>
#include <FL/Fl_RGB_Image.H>
#include "totype.hpp"
#include "fflerror.hpp"

class ImageCache
{
    private:
        std::unordered_map<uint32_t, std::unique_ptr<Fl_Image>> m_cache;

    public:
        ImageCache() = default;

    public:
        Fl_Image *Retrieve(uint8_t fileIndex, uint16_t imageIndex) const
        {
            if(auto p = m_cache.find((to_u32(fileIndex) << 16) + imageIndex); p != m_cache.end()){
                return p->second.get();
            }
            return nullptr;
        }

    public:
        void Register(uint8_t fileIndex, uint16_t imageIndex, const void *data, int w, int h)
        {
            fflassert(data);
            fflassert(w > 0);
            fflassert(h > 0);

            // understand the Fl_Image and Fl_RGB_Image:
            // Fl_RGB_Image needs a RGBA buffer as constructor parameter, but it won't create its internal copy
            // caller has to make sure the buffer is available and not changed

            // but Fl_Image doesn't require such a buffer
            // immediately after Fl_RGB_Image::copy() returns the Fl_Image pointer
            // we are good to use the RGBA buffer for other purpose

            m_cache[(to_u32(fileIndex) << 16) + imageIndex].reset(Fl_RGB_Image
            {
                static_cast<const uchar *>(data),
                w,
                h,
                4,
            }.copy());
        }
};

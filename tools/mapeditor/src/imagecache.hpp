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
#include <string>
#include <unordered_map>
#include <FL/Fl_Shared_Image.H>

class ImageCache
{
    private:
        std::string m_Path;
        std::unordered_map<uint32_t, Fl_Shared_Image *> m_Cache;

    public:
        ImageCache();
       ~ImageCache();

    public:
        Fl_Shared_Image *Retrieve(uint8_t, uint16_t);
        bool Register(uint8_t, uint16_t, const uint32_t *, int, int);

    public:
        void SetPath(const char *);
        const std::string &Path()
        {
            return m_Path;
        }
};

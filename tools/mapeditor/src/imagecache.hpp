/*
 * =====================================================================================
 *
 *       Filename: imagecache.hpp
 *        Created: 02/14/2016 15:40:17
 *  Last Modified: 02/15/2016 02:10:53
 *
 *    Description: Interaction with WilImagePackage
 *                 return drawable objects with ImageKey provided by EditorMap
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
#include <FL/Fl_Shared_Image.H>
#include <unordered_map>



class ImageCache
{
    public:
        ImageCache();
        ~ImageCache();

    public:
        Fl_Shared_Image *Retrieve(uint8_t, uint16_t);
        bool Register(uint8_t, uint16_t, uint32_t *, int, int);

    public:
        void SetPath(const char *);

    private:
        std::string m_Path;
        std::unordered_map<uint32_t, Fl_Shared_Image *> m_Cache;
};

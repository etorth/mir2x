/*
 * =====================================================================================
 *
 *       Filename: magicframedb.hpp
 *        Created: 07/26/2017 04:27:57
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
#include <FL/Fl_PNG_Image.H>
#include "zsdb.hpp"

class MagicFrameDB final
{
    private:
        struct CachedFrame
        {
            std::unique_ptr<Fl_PNG_Image> image;
            int dx;
            int dy;
        };

    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;
        std::unordered_map<uint32_t, CachedFrame> m_cachedFrameList;

    public:
        MagicFrameDB(const char *zsdbPath)
            : m_zsdbPtr(std::make_unique<ZSDB>(zsdbPath))
        {}

    public:
        std::tuple<Fl_Image *, int, int> retrieve(uint32_t);
};

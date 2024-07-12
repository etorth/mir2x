#pragma once
#include <FL/Fl_PNG_Image.H>
#include "zsdb.hpp"

class MagicFrameDB final
{
    private:
        struct CachedFrame
        {
            int dx = 0;
            int dy = 0;
            std::unique_ptr<Fl_PNG_Image> image = nullptr;
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

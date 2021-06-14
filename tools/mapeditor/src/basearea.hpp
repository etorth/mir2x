/*
 * =====================================================================================
 *
 *       Filename: basearea.hpp
 *        Created: 07/26/2015 04:27:57
 *    Description: derived class from Fl_Box
 *                 support cropped drawing for all primitives
 *                 used as base class for MainDrawArea and LayerViewArea
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
#include <map>
#include <memory>
#include <vector>
#include <cstdint>
#include <optional>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include "totype.hpp"

class BaseArea: public Fl_Box
{
    private:
        struct ColorStackEntry
        {
            Fl_Color Color;
            unsigned Count;

            ColorStackEntry(Fl_Color stColor = FL_WHITE, unsigned nCount = 1)
                : Color(stColor)
                , Count(nCount)
            {}
        };

    private:
        std::vector<ColorStackEntry> m_colorStack;

    private:
        std::map<uint32_t, std::shared_ptr<Fl_Image>> m_coverRecord;

    public:
        BaseArea(int, int, int, int);

    public:
        ~BaseArea() = default;

    public:
        virtual std::tuple<int, int> offset() const = 0;

    public:
        void drawImage(Fl_Image *, int, int);
        void drawImage(Fl_Image *, int, int, int, int, int, int);

    public:
        void DrawText(int, int, const char *, ...);
        void drawImageCover(Fl_Image *, int, int, int, int);

    public:
        void DrawCircle(int, int, int);

    public:
        void drawLine(int, int, int, int);
        void DrawLoop(int, int, int, int, int, int);
        void drawRectangle(int, int, int, int);

    public:
        void PushColor(Fl_Color);
        void PopColor();

    public:
        void clear();

    public:
        void FillRectangle(int, int, int, int, uint32_t);

    private:
        Fl_Image *RetrieveImageCover(uint32_t);
        Fl_Image *CreateImageCover(int, int, uint32_t);

    public:
        virtual std::optional<std::tuple<size_t, size_t>> getROISize() const = 0;

    public:
        std::tuple<size_t, size_t> getScrollPixelCount() const
        {
            const auto roiSize = getROISize();
            if(!roiSize.has_value()){
                return {0, 0};
            }

            const auto [xpCount, ypCount] = roiSize.value();
            return
            {
                [xpCount, this]() -> size_t
                {
                    if(to_d(xpCount) > w()){
                        return to_d(xpCount) - w();
                    }
                    return 0;
                }(),

                [ypCount, this]() -> float
                {
                    if(to_d(ypCount) > h()){
                        return to_d(ypCount) - h();
                    }
                    return 0;
                }(),
            };
        }

        std::tuple<float, float> getScrollPixelRatio(int dx, int dy) const
        {
            const auto [xpCount, ypCount] = getScrollPixelCount();
            return
            {
                (xpCount > 0) ? to_f(dx) / xpCount : 0.0,
                (ypCount > 0) ? to_f(dy) / ypCount : 0.0,
            };
        }
};

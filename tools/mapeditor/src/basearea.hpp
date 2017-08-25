/*
 * =====================================================================================
 *
 *       Filename: basearea.hpp
 *        Created: 07/26/2015 04:27:57
 *  Last Modified: 08/24/2017 14:30:06
 *
 *    Description: derived class from Fl_Box
 *                 support cropped drawing for all primitives
 *                 used as base class for MainDrawArea and LayerEditArea
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
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>

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
        std::vector<ColorStackEntry> m_ColorStack;

    private:
        std::map<uint32_t, std::shared_ptr<Fl_Image>> m_CoverRecord;

    public:
        BaseArea(int, int, int, int);

    public:
        ~BaseArea() = default;

    public:
        void DrawImage(Fl_Image *, int, int);
        void DrawImage(Fl_Image *, int, int, int, int, int, int);

    public:
        void DrawText(int, int, const char *, ...);
        void DrawImageCover(Fl_Image *, int, int, int, int);

    public:
        void DrawCircle(int, int, int);

    public:
        void DrawLine(int, int, int, int);
        void DrawLoop(int, int, int, int, int, int);
        void DrawRectangle(int, int, int, int);

    public:
        void PushColor(Fl_Color);
        void PopColor();

    public:
        void Clear();

    public:
        void FillRectangle(int, int, int, int, uint32_t);

    private:
        Fl_Image *RetrieveImageCover(uint32_t);
        Fl_Image *CreateImageCover(int, int, uint32_t);
};

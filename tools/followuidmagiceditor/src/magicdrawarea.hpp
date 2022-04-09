/*
 * =====================================================================================
 *
 *       Filename: magicdrawarea.hpp
 *        Created: 07/26/2015 04:27:57
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
#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <cstdint>
#include <unordered_map>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include "magicframedb.hpp"
#include "dbcomid.hpp"

class MagicDrawArea: public Fl_Box
{
    private:
        bool m_adjustR = false;
        int  m_adjustTargetOff = -1;

    private:
        uint32_t m_r = 160;
        uint32_t m_frame = 0;

    private:
        uint32_t m_magicID;
        std::unique_ptr<MagicFrameDB> m_frameDBPtr;

    private:
        std::vector<std::tuple<int, int>> m_offList;

    public:
        MagicDrawArea(int, int, int, int);

    public:
        ~MagicDrawArea() = default;

    public:
        void drawImage(Fl_Image *, int, int);
        void drawImage(Fl_Image *, int, int, int, int, int, int);

    public:
        void drawText(int, int, Fl_Color, const char *, ...);

    public:
        void drawCircle(Fl_Color, int, int, int);

    public:
        void drawLine(Fl_Color, int, int, int, int);
        void drawRectangle(Fl_Color, int, int, int, int);

    public:
        void Clear();

    public:
        void draw();
        int  handle(int);

    public:
        void load(uint32_t, const char *);

    public:
        void reset();
        void output() const;

    public:
        void clear();
        void updateFrame();

    private:
        std::tuple<int, int> getGfxDirPLoc(int) const;
        std::tuple<Fl_Image *, int, int> getFrameImage(int);

    private:
        int magicDirCount() const
        {
            return DBCOM_MAGICGFXENTRY(m_magicID, u8"运行").first->gfxDirType;
        }
};

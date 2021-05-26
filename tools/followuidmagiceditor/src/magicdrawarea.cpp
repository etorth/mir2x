/*
 * =====================================================================================
 *
 *       Filename: magicdrawarea.cpp
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

#include <cmath>
#include <cstdio>
#include <string>
#include <cstdarg>
#include <cstring>
#include <algorithm>
#include <cinttypes>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include "strf.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "flwrapper.hpp"
#include "dbcomrecord.hpp"
#include "magicdrawarea.hpp"

MagicDrawArea::MagicDrawArea(int argX, int argY, int argW, int argH)
    : Fl_Box(argX, argY, argW, argH)
{}

void MagicDrawArea::drawImage(Fl_Image *image, int dstX, int dstY)
{
    if(image){
        drawImage(image, dstX, dstY, 0, 0, image->w(), image->h());
    }
}

void MagicDrawArea::drawImage(Fl_Image *image, int dstX, int dstY, int srcX, int srcY, int srcW, int srcH)
{
    if(true
            && w() > 0
            && h() > 0

            && image
            && image->w() > 0
            && image->h() > 0){

        const int srcXOld = srcX;
        const int srcYOld = srcY;

        if(true
                && srcW > 0
                && srcH > 0
                && mathf::rectangleOverlapRegion<int>(0, 0, image->w(), image->h(), &srcX, &srcY, &srcW, &srcH)){

            dstX += (srcX - srcXOld);
            dstY += (srcY - srcYOld);

            const auto dstXOld = dstX;
            const auto dstYOld = dstY;

            if(true
                    && srcW > 0
                    && srcH > 0
                    && mathf::rectangleOverlapRegion<int>(0, 0, w(), h(), &dstX, &dstY, &srcW, &srcH)){

                srcX += (dstX - dstXOld);
                srcY += (dstY - dstYOld);

                if(true
                        && srcW > 0
                        && srcH > 0){
                    image->draw(dstX + x(), dstY + y(), srcW, srcH, srcX, srcY);
                }
            }
        }
    }
}

void MagicDrawArea::drawText(int argX, int argY, Fl_Color color, const char *format, ...)
{
    std::string text;
    str_format(format, text);

    fl_wrapper::enable_color enable(color);
    fl_draw(text.c_str(), argX + x(), argY + y(), x() + w(), y() + h(), FL_ALIGN_TOP_LEFT, nullptr, 0);
}

void MagicDrawArea::drawCircle(Fl_Color color, int argX, int argY, int argR)
{
    fl_wrapper::enable_color enable(color);
    fl_circle(x() + argX, y() + argY, argR);
}

void MagicDrawArea::drawLine(Fl_Color color, int argX0, int argY0, int argX1, int argY1)
{
    if(mathf::locateLineSegment(0, 0, w(), h(), &argX0, &argY0, &argX1, &argY1)){
        fl_wrapper::enable_color enable(color);
        fl_line(argX0 + x(), argY0 + y(), argX1 + x(), argY1 + y());
    }
}

void MagicDrawArea::drawRectangle(Fl_Color color, int argX, int argY, int argW, int argH)
{
    fl_wrapper::enable_color enable(color);
    fl_rect(x() + argX, y() + argY, argW, argH);
}

void MagicDrawArea::clear()
{
    fl_rectf(x(), y(), w(), h(), 0, 0, 0);
}

std::tuple<Fl_Image *, int, int> MagicDrawArea::getFrameImage(int gfxDirIndex)
{
    const auto &gfxEntry = DBCOM_MAGICRECORD(m_magicID).getGfxEntry(u8"运行");
    fflassert(gfxEntry);

    if(gfxEntry.gfxID == SYS_TEXNIL){
        return {nullptr, 0, 0};
    }
    return m_frameDBPtr->retrieve(gfxEntry.gfxID + m_frame + gfxDirIndex * gfxEntry.gfxIDCount);
}

void MagicDrawArea::draw()
{
    Fl_Box::draw();

    const int centerX = w() / 2;
    const int centerY = h() / 2;
    drawCircle(FL_RED, centerX, centerY, m_r);

    constexpr float angle16 = 2.0 * 3.1415926535 / 16.0;
    for(int i = 0; i < 16; ++i){
        const float dstX = to_f(m_r) * std::cos(i * angle16);
        const float dstY = to_f(m_r) * std::sin(i * angle16);
        drawLine(FL_BLUE, centerX, centerY, centerX + dstX, centerY + dstY);
    }

    for(int i = 0; i < 16; ++i){
        const float dstX = to_f(m_r) * std::cos(i * angle16);
        const float dstY = to_f(m_r) * std::sin(i * angle16);
        const auto [image, dx, dy] = getFrameImage(i);

        drawImage(image, dstX + dx + centerX, dstY + dy + centerY);
        drawRectangle(FL_BLUE, dstX + dx + centerX, dstY + dy + centerY, image->w(), image->h());
        drawLine(FL_RED, dstX + centerX, dstY + centerY, dstX + centerX + dx, dstY + centerY + dy);

        const auto [txOff, tyOff] = m_offList.at(i);
        drawLine(FL_RED, dstX + centerX, dstY + centerY, dstX + centerX + txOff, dstY + centerY + tyOff);
    }
}

int MagicDrawArea::handle(int event)
{
    return Fl_Box::handle(event);
}

void MagicDrawArea::load(uint32_t magicID, const char *dbPathName)
{
    m_magicID = magicID;
    m_frameDBPtr = std::make_unique<MagicFrameDB>(dbPathName);
    m_offList.resize(DBCOM_MAGICRECORD(magicID).getGfxEntry(u8"运行").gfxDirType);
}

void MagicDrawArea::reset()
{
    m_r = 160;
    std::fill(m_offList.begin(), m_offList.end(), std::tuple<int, int>(0, 0));
}

void MagicDrawArea::output() const
{
    const auto &mr = DBCOM_MAGICRECORD(m_magicID);
    std::fprintf(stdout, R"###(case DBCOM_MAGICID(u8"%s"):\n)###", to_cstr(mr.name));
}

void MagicDrawArea::updateFrame()
{
    m_frame = (m_frame + 1) % DBCOM_MAGICRECORD(m_magicID).getGfxEntry(u8"运行").frameCount;
}

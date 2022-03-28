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
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include "strf.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "flwrapper.hpp"
#include "dbcomid.hpp"
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
                && mathf::rectangleOverlapRegion<int>(0, 0, image->w(), image->h(), srcX, srcY, srcW, srcH)){

            dstX += (srcX - srcXOld);
            dstY += (srcY - srcYOld);

            const auto dstXOld = dstX;
            const auto dstYOld = dstY;

            if(true
                    && srcW > 0
                    && srcH > 0
                    && mathf::rectangleOverlapRegion<int>(0, 0, w(), h(), dstX, dstY, srcW, srcH)){

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
    const auto &gfxEntry = DBCOM_MAGICRECORD(m_magicID).getGfxEntry(u8"运行").first;
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
    drawCircle(m_adjustR ? FL_RED : FL_GREEN, centerX, centerY, m_r);

    for(int i = 0; i < magicDirCount(); ++i){
        const auto [dstX, dstY] = getGfxDirPLoc(i);
        drawLine(FL_BLUE, centerX, centerY, dstX, dstY);
    }

    for(int i = 0; i < magicDirCount(); ++i){
        const auto [dstX, dstY] = getGfxDirPLoc(i);
        const auto [image, dx, dy] = getFrameImage(i);

        fflassert(image);
        drawImage(image, dstX + dx, dstY + dy);
        drawRectangle(FL_BLUE, dstX + dx, dstY + dy, image->w(), image->h());
        drawLine(FL_RED, dstX, dstY, dstX + dx, dstY + dy);

        const auto [txOff, tyOff] = m_offList.at(i);
        drawLine(FL_RED, dstX, dstY, dstX + txOff, dstY + tyOff);
    }

    if(m_adjustTargetOff >= 0){
        const auto mouseX = Fl::event_x() - x();
        const auto mouseY = Fl::event_y() - y();
        const auto [dstX, dstY] = getGfxDirPLoc(m_adjustTargetOff);

        drawLine(FL_MAGENTA, dstX, dstY, mouseX, mouseY);
        drawLine(FL_MAGENTA, mouseX - 8, mouseY, mouseX + 8, mouseY);
        drawLine(FL_MAGENTA, mouseX, mouseY - 8, mouseX, mouseY + 8);
    }
}

int MagicDrawArea::handle(int event)
{
    auto result = Fl_Box::handle(event);
    const auto mouseX = Fl::event_x() - x();
    const auto mouseY = Fl::event_y() - y();

    switch(event){
        case FL_DRAG:
            {
                if(m_adjustR){
                    m_r = mathf::LDistance<int>(w() / 2, h() / 2, mouseX, mouseY);
                }
                break;
            }
        case FL_PUSH:
            {
                if(Fl::event_state() & FL_CTRL){
                    m_adjustR = true;
                    fl_cursor(FL_CURSOR_MOVE);
                }
                else if(Fl::event_clicks() >= 1){
                    for(int i = 0; i < magicDirCount(); ++i){
                        const auto [dstX, dstY] = getGfxDirPLoc(i);
                        const auto [image, dx, dy] = getFrameImage(i);

                        fflassert(image);
                        const auto boxX = dstX + dx;
                        const auto boxY = dstY + dy;

                        if(mathf::pointInRectangle(mouseX, mouseY, boxX, boxY, image->w(), image->h())){
                            m_adjustTargetOff = i;
                            break;
                        }
                    }
                }
                else if(m_adjustTargetOff >= 0){
                    if(Fl::event_state() & FL_BUTTON1){
                        const auto [dstX, dstY] = getGfxDirPLoc(m_adjustTargetOff);
                        m_offList.at(m_adjustTargetOff) = std::tuple<int, int>(mouseX - dstX, mouseY - dstY);
                    }
                    m_adjustTargetOff = -1;
                }

                result = 1;
                break;
            }
        case FL_RELEASE:
            {
                m_adjustR = false;
                fl_cursor(FL_CURSOR_DEFAULT);
                break;
            }
        default:
            {
                break;
            }
    }
    return result;
}

void MagicDrawArea::load(uint32_t magicID, const char *dbPathName)
{
    m_magicID = magicID;
    m_frameDBPtr = std::make_unique<MagicFrameDB>(dbPathName);
    m_offList.resize(magicDirCount());
}

void MagicDrawArea::reset()
{
    m_r = 160;
    std::fill(m_offList.begin(), m_offList.end(), std::tuple<int, int>(0, 0));
}

void MagicDrawArea::output() const
{
    std::fprintf(stdout, "case DBCOM_MAGICID(u8\"%s\"):\n", to_cstr(DBCOM_MAGICRECORD(m_magicID).name));
    std::fprintf(stdout, "{\n");
    std::fprintf(stdout, "switch(gfxDirIndex()){\n");

    for(int i = 0; const auto &[dx, dy]: m_offList){
        std::fprintf(stdout, "case %2d: return {%3d, %3d};\n", i++, dx, dy);
    }

    std::fprintf(stdout, "default: throw bad_reach();\n");
    std::fprintf(stdout, "}\n");
    std::fprintf(stdout, "}\n");
}

void MagicDrawArea::updateFrame()
{
    m_frame = (m_frame + 1) % DBCOM_MAGICRECORD(m_magicID).getGfxEntry(u8"运行").first.frameCount;
}

std::tuple<int, int> MagicDrawArea::getGfxDirPLoc(int gfxDirIndex) const
{
    fflassert(gfxDirIndex >= 0);

    constexpr float pi = 3.1415926535;
    constexpr float angle16 = 2.0 * pi / 16.0;

    const float gfxDirAngle = pi / 2.0 - gfxDirIndex * angle16;
    return
    {
        to_d(w() / 2.0 + to_f(m_r) * std::cos(gfxDirAngle)),
        to_d(h() / 2.0 - to_f(m_r) * std::sin(gfxDirAngle)),
    };
}

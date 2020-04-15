/*
 * =====================================================================================
 *
 *       Filename: basearea.cpp
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

#include <string>
#include <cstdarg>
#include <cstring>
#include <cinttypes>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include "mathf.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "drawarea.hpp"

BaseArea::BaseArea(int nX, int nY, int nW, int nH)
    : Fl_Box(nX, nY, nW, nH)
    , m_ColorStack()
    , m_CoverRecord()
{
    PushColor(FL_RED);
}

void BaseArea::DrawImage(Fl_Image *pImage, int nX, int nY)
{
    if(pImage){
        DrawImage(pImage, nX, nY, 0, 0, pImage->w(), pImage->h());
    }
}

void BaseArea::DrawImage(Fl_Image *pImage, int nX, int nY, int nImageX, int nImageY, int nImageW, int nImageH)
{
    if(true
            && w() > 0
            && h() > 0

            && pImage
            && pImage->w() > 0
            && pImage->h() > 0){

        int nOldImageX = nImageX;
        int nOldImageY = nImageY;

        if(true
                && nImageW > 0
                && nImageH > 0
                && mathf::rectangleOverlapRegion<int>(0, 0, pImage->w(), pImage->h(), &nImageX, &nImageY, &nImageW, &nImageH)){

            nX += (nImageX - nOldImageX);
            nY += (nImageY - nOldImageY);

            auto nOldAX = nX;
            auto nOldAY = nY;

            if(true
                    && nImageW > 0
                    && nImageH > 0
                    && mathf::rectangleOverlapRegion<int>(0, 0, w(), h(), &nX, &nY, &nImageW, &nImageH)){

                nImageX += (nX - nOldAX);
                nImageY += (nY - nOldAY);

                if(true
                        && nImageW > 0
                        && nImageH > 0){
                    pImage->draw(nX + x(), nY + y(), nImageW, nImageH, nImageX, nImageY);
                }
            }
        }
    }
}

void BaseArea::DrawText(int nX, int nY, const char *szLogFormat, ...)
{
    auto fnDraw = [this](int nX, int nY, const char *szLogInfo)
    {
        if(true
                && szLogInfo
                && std::strlen(szLogInfo)){

            // need to draw inside draw area
            // use the most fancy function to draw text
            fl_draw(szLogInfo, nX + x(), nY + y(), x() + w(), y() + h(), FL_ALIGN_TOP_LEFT, nullptr, 0);
        }
    };

    int nLogSize = 0;

    // 1. try static buffer
    //    give an enough size so we can hopefully stop here
    {
        char szSBuf[1024];

        va_list ap;
        va_start(ap, szLogFormat);
        nLogSize = std::vsnprintf(szSBuf, (sizeof(szSBuf) / sizeof(szSBuf[0])), szLogFormat, ap);
        va_end(ap);

        if(nLogSize >= 0){
            if((size_t)(nLogSize + 1) < (sizeof(szSBuf) / sizeof(szSBuf[0]))){
                fnDraw(nX, nY, szSBuf);
                return;
            }else{
                // do nothing
                // have to try the dynamic buffer method
            }
        }else{
            fnDraw(nX, nY, (std::string("Parse message failed: ") + szLogFormat).c_str());
            return;
        }
    }

    // 2. try dynamic buffer
    //    use the parsed buffer size above to get enough memory
    while(true){
        std::vector<char> szDBuf(nLogSize + 1 + 64);

        va_list ap;
        va_start(ap, szLogFormat);
        nLogSize = std::vsnprintf(&(szDBuf[0]), szDBuf.size(), szLogFormat, ap);
        va_end(ap);

        if(nLogSize >= 0){
            if((size_t)(nLogSize + 1) < szDBuf.size()){
                fnDraw(nX, nY, &(szDBuf[0]));
                return;
            }else{
                szDBuf.resize(nLogSize + 1 + 64);
            }
        }else{
            fnDraw(nX, nY, (std::string("Parse message failed: ") + szLogFormat).c_str());
            return;
        }
    }
}

void BaseArea::DrawImageCover(Fl_Image *pImage, int nX, int nY, int nW, int nH)
{
    if(true
            && pImage
            && pImage->w() > 0
            && pImage->h() > 0

            && nW > 0
            && nH > 0
            && mathf::rectangleOverlapRegion(0, 0, w(), h(), &nX, &nY, &nW, &nH)){

        // use an image as a cover to repeat
        // should do partically drawing at end of x and y

        int nGXCnt = nW / pImage->w();
        int nGYCnt = nH / pImage->h();
        int nGXRes = nW % pImage->w();
        int nGYRes = nH % pImage->h();

        for(int nGX = 0; nGX < nGXCnt; ++nGX){
            for(int nGY = 0; nGY < nGYCnt; ++nGY){
                DrawImage(pImage, nX + nGX * pImage->w(), nY + nGY * pImage->h());
            }
        }

        if(nGXRes > 0){
            for(int nGY = 0; nGY < nGYCnt; ++nGY){
                DrawImage(pImage,
                        nX + nGXCnt * pImage->w(),
                        nY + nGY    * pImage->h(),
                        0,
                        0,
                        nGXRes,
                        pImage->h());
            }
        }

        if(nGYRes > 0){
            for(int nGX = 0; nGX < nGXCnt; ++nGX){
                DrawImage(pImage,
                        nX + nGX    * pImage->w(),
                        nY + nGYCnt * pImage->h(),
                        0,
                        0,
                        pImage->w(),
                        nGYRes);
            }
        }

        if(true
                && nGXRes > 0
                && nGYRes > 0){

            DrawImage(pImage,
                    nX + nGXCnt * pImage->w(),
                    nY + nGYCnt * pImage->h(),
                    0,
                    0,
                    nGXRes,
                    nGYRes);
        }
    }
}

void BaseArea::DrawCircle(int nX, int nY, int nR)
{
    if(nR > 0){
        int nCBoxX = nX - nR - 1;
        int nCBoxY = nY - nR - 1;
        int nCBoxW =  2 * nR - 1;

        // draw a full circle
        if(mathf::rectangleInside(0, 0, w(), h(), nCBoxX, nCBoxY, nCBoxW, nCBoxW)){
            fl_circle(x() + nX, y() + nY, nR);
            return;
        }

        // TODO
        // draw part of a circle
    }
}

void BaseArea::DrawLine(int nX0, int nY0, int nX1, int nY1)
{
    if(mathf::locateLineSegment(0, 0, w(), h(), &nX0, &nY0, &nX1, &nY1)){
        fl_line(nX0 + x(), nY0 + y(), nX1 + x(), nY1 + y());
    }
}

void BaseArea::DrawLoop(int nX1, int nY1, int nX2, int nY2, int nX3, int nY3)
{
    DrawLine(nX1, nY1, nX2, nY2);
    DrawLine(nX2, nY2, nX3, nY3);
    DrawLine(nX3, nY3, nX1, nY1);
}

void BaseArea::DrawRectangle(int nX, int nY, int nW, int nH)
{
    DrawLine(nX         , nY         , nX + nW - 1, nY         );
    DrawLine(nX         , nY         , nX         , nY + nH - 1);
    DrawLine(nX + nW - 1, nY         , nX + nW - 1, nY + nH - 1);
    DrawLine(nX         , nY + nH - 1, nX + nW - 1, nY + nH - 1);
}

void BaseArea::PushColor(Fl_Color stColor)
{
    if(m_ColorStack.empty() || stColor != m_ColorStack.back().Color){
        fl_color(stColor);
        m_ColorStack.push_back({stColor, 1});
    }else{
        // have to put it here to reset
        // fl_color() is global and other class may call it
        fl_color(stColor);
        m_ColorStack.back().Count++;
    }
}

void BaseArea::PopColor()
{
    if(m_ColorStack.empty()){
        PushColor(FL_WHITE);
    }else{
        if(m_ColorStack.back().Count <= 0){
            throw fflerror("color stack empty");
        }
        if(m_ColorStack.back().Count == 1){
            m_ColorStack.pop_back();
            if(m_ColorStack.empty()){
                PushColor(FL_WHITE);
            }else{
                fl_color(m_ColorStack.back().Color);
            }
        }else{
            m_ColorStack.back().Count--;

            // have to put it here to reset
            // fl_color() is global and other class may call it
            fl_color(m_ColorStack.back().Color);
        }
    }
}

void BaseArea::Clear()
{
    fl_rectf(x(), y(), w(), h(), 0, 0, 0);
}

void BaseArea::FillRectangle(int nX, int nY, int nW, int nH, uint32_t nARGB)
{
    if(true
            && nW > 0
            && nH > 0
            && mathf::rectangleOverlap(0, 0, w(), h(), nX, nY, nW, nH)
            && ((nARGB & 0XFF000000))){

        if(auto pImage = RetrieveImageCover(nARGB)){
            DrawImageCover(pImage, nX, nY, nW, nH);
        }
    }
}

Fl_Image *BaseArea::CreateImageCover(int nW, int nH, uint32_t nColor)
{
    if(true
            && nW > 0
            && nH > 0){
        std::vector<uint32_t> stvBuf(nW * nH, nColor);
        return Fl_RGB_Image((uchar *)(&(stvBuf[0])), nW, nH, 4, 0).copy(nW, nH);
    }else{
        fl_alert("Invalid size for CreateImageCover(%d, %d, 0X%08" PRIu32 ")", nW, nH, nColor);
        return nullptr;
    }
}

Fl_Image *BaseArea::RetrieveImageCover(uint32_t nARGB)
{
    auto pRecord = m_CoverRecord.find(nARGB);
    if(pRecord != m_CoverRecord.end()){
        return pRecord->second.get();
    }else{
        auto pImage = CreateImageCover(SYS_MAPGRIDXP, SYS_MAPGRIDYP, nARGB);
        m_CoverRecord[nARGB].reset(pImage);
        return pImage;
    }
}

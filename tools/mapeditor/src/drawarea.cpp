/*
 * =====================================================================================
 *
 *       Filename: drawarea.cpp
 *        Created: 7/26/2015 4:27:57 AM
 *  Last Modified: 02/10/2016 19:05:54
 *
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

#include "supwarning.hpp"
#include "drawarea.hpp"
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_RGB_Image.H>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <vector>
#include <string>
#include "mir2map.hpp"
#include <algorithm>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <functional>
#include "mainwindow.hpp"
#include "groundinfowindow.hpp"
#include "mathfunc.hpp"

DrawArea::DrawArea(int x, int y, int w, int h)
    : Fl_Box(x, y, w, h)
    , m_MouseX(0)
    , m_MouseY(0)
    , m_OffsetX(0)
    , m_OffsetY(0)
{
}

DrawArea::~DrawArea()
{
}

void DrawArea::draw()
{
    Fl_Box::draw();
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ClearBackground()){
        fl_rectf(x(), y(), w(), h(), 0, 0, 0);
    }

    extern Mir2Map g_Map;
    if(!g_Map.Valid()){
        return;
    }
    DrawBaseTile();
    DrawGroundObject();
    DrawOverGroundObject();
	DrawGroundInfo();

    DrawSelect();
    DrawTextBox();
}

void DrawArea::DrawTriangleUnderCover()
{
    int nMOMX = m_MouseX - x() + m_OffsetX; // mouse on map
    int nMOMY = m_MouseY - y() + m_OffsetY; //

    extern MainWindow *g_MainWindow;
    int nR = g_MainWindow->SelectCoverRadius();

    int nStartX = (nMOMX - g_MainWindow->SelectCoverRadius() - 48 / 2) / 48;
    int nStartY = (nMOMY - g_MainWindow->SelectCoverRadius() - 32 / 2) / 32;
    int nStopX  = (nMOMX + g_MainWindow->SelectCoverRadius() + 48 / 2) / 48;
    int nStopY  = (nMOMY + g_MainWindow->SelectCoverRadius() + 32 / 2) / 32;

    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    for(int nX = nStartX; nX <= nStopX; ++nX){
        for(int nY = nStartY; nY <= nStopY; ++nY){
            for(int nIndex = 0; nIndex < 4; ++nIndex){
                int nMidX, nMidY, nX1, nY1, nX2, nY2;
                GetTriangleOnMap(nX, nY, nIndex, nMidX, nMidY, nX1, nY1, nX2, nY2);
                if(PointInCircle(nMidX, nMidY, nMOMX, nMOMY, nR)
                        || PointInCircle(nX1, nY1, nMOMX, nMOMY, nR)
                        || PointInCircle(nX2, nY2, nMOMX, nMOMY, nR)){
                    // fl_loop(nMidX + nDX, nMidY + nDY, nX1 + nDX, nY1 + nDY, nX2 + nDX, nY2 + nDY);
                    UNUSED(nDX); UNUSED(nDY);
                    DrawTriangleUnit(nX, nY, nIndex);
                }
            }
        }
    }
}

void DrawArea::DrawSelect()
{
    extern MainWindow *g_MainWindow;
    auto wColor = fl_color();
    fl_color(FL_RED);

    if(g_MainWindow->EnableSelect() && g_MainWindow->SelectByCover()){
        DrawTriangleUnderCover();
        fl_circle(m_MouseX, m_MouseY, g_MainWindow->SelectCoverRadius());
    }

    if(g_MainWindow->EnableSelect() && g_MainWindow->SelectByRegion()){

        extern std::vector<std::pair<int, int>> g_SelectByRegionPointV;

        int nDX = x() - m_OffsetX;
        int nDY = y() - m_OffsetY;

        if(g_SelectByRegionPointV.size() > 1){
            for(size_t nIndex = 0; nIndex < g_SelectByRegionPointV.size() - 1; ++nIndex){
                fl_line(g_SelectByRegionPointV[nIndex].first + nDX, g_SelectByRegionPointV[nIndex].second + nDY,
                        g_SelectByRegionPointV[nIndex + 1].first + nDX, g_SelectByRegionPointV[nIndex + 1].second + nDY);
            }
        }

        if(!g_SelectByRegionPointV.empty()){
            for(auto &stPair: g_SelectByRegionPointV){
                fl_circle(stPair.first + nDX, stPair.second +nDY, 4.0);
            }
            fl_line(m_MouseX, m_MouseY,
                    g_SelectByRegionPointV.back().first + nDX,
                    g_SelectByRegionPointV.back().second + nDY);
        }

        // // can't do alpha blend, so comment this out
        // fl_color(FL_YELLOW);
        // if(g_SelectByRegionPointV.size() > 2){
        //     for(size_t nIndex = 1; nIndex < g_SelectByRegionPointV.size() - 1; ++nIndex){
        //         fl_polygon(g_SelectByRegionPointV[0].first, g_SelectByRegionPointV[0].second,
        //                 g_SelectByRegionPointV[nIndex].first, g_SelectByRegionPointV[nIndex].second,
        //                 g_SelectByRegionPointV[nIndex + 1].first, g_SelectByRegionPointV[nIndex + 1].second);
        //     }
        // }
    }

    DrawCover();

    fl_color(wColor);
}

void DrawArea::DrawCover()
{
    extern MainWindow *g_MainWindow;
    int nR = g_MainWindow->SelectCoverRadius();
    if(nR == 0){
        return;
    }

    static Fl_RGB_Image *pCircle = nullptr;
    static int nOldR = 0;

    if(pCircle == nullptr || nOldR != nR){
        uint32_t *pData = new uint32_t[nR * nR * 4];
        uint32_t  nCB = 0X00000000;
        uint32_t  nCF = 0X100000AA;
        for(int nX = 0; nX < nR * 2; ++nX){
            for(int nY = 0; nY < nR * 2; ++nY){
                if((nX - nR) * (nX - nR) + (nY - nR) * (nY - nR) < nR * nR){
                    pData[nY * 2* nR + nX] = nCF;
                }else{
                    pData[nY * 2* nR + nX] = nCB;
                }
            }
        }
        delete pCircle;
        pCircle = new Fl_RGB_Image((uchar *)pData, nR, nR, 4, 0);
        delete pData;
        nOldR = nR;
    }

    if(pCircle){
        pCircle->draw(m_MouseX - nR, m_MouseY - nR);
    }

    // {
    //     static Fl_Shared_Image *pCircle = nullptr;
    //     if(pCircle == nullptr){
    //         pCircle = Fl_Shared_Image::get("/home/anhong/Dropbox/alphacircle.png");
    //     }
    //     if(pCircle){
    //         pCircle->draw(m_MouseX - pCircle->w() / 2, m_MouseY - pCircle->h() / 2);
    //     }
    // }
}

void DrawArea::DrawTextBox()
{
    // fl_rectf(x(), y(), 48 * 4, 32 * 5, 0, 0, 0);
    // extern MainWindow *g_MainWindow;
    {
        static Fl_Shared_Image *pTextBoxBG = nullptr;
        if(pTextBoxBG == nullptr){
            pTextBoxBG = Fl_Shared_Image::get("/home/anhong/Dropbox/texboxbg.png");
        }
        if(pTextBoxBG){
            pTextBoxBG->draw(x(), y());
        }
    }

    auto wColor = fl_color();
    fl_color(FL_RED);

    char szInfo[128];
    std::sprintf(szInfo, "OffsetX: %d %d", m_OffsetX / 48, m_OffsetX);
    fl_draw(szInfo, 10 + x(), 20 + y());

    std::sprintf(szInfo, "OffsetY: %d %d", m_OffsetY / 32, m_OffsetY);
    fl_draw(szInfo, 10 + x(), 40 + y());

    int nDX = m_OffsetX - x();
    int nDY = m_OffsetY - y();

    std::sprintf(szInfo, "MouseMX: %d %d", (m_MouseX + nDX) / 48, (m_MouseX + nDX));
    fl_draw(szInfo, 10 + x(), 60 + y());

    std::sprintf(szInfo, "MouseMY: %d %d", (m_MouseY + nDY) / 32, (m_MouseY + nDY));
    fl_draw(szInfo, 10 + x(), 80 + y());

    fl_color(wColor);
}

void DrawArea::DrawGroundObject()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowGroundObjectLayer()){ return; }

    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    auto fnDrawObjFunc = [this, nDX, nDY](uint32_t nFolderIndex, uint32_t nImageIndex, Fl_Shared_Image * pImage, int nXCnt, int nYCnt){
        extern MainWindow *g_MainWindow;
        // auto p = g_MainWindow->RetrievePNG(nFolderIndex, nImageIndex);
        auto p = pImage;
        if(!p){
            p = g_MainWindow->RetrieveCachedPNG(nFolderIndex, nImageIndex, nXCnt, nYCnt);
        }
        if(p){
            // int nStartX = nXCnt * 48 - 200;
            // int nStartY = nYCnt * 32 - 157 + 32 - p->h();
            int nStartX = nXCnt * 48;
            int nStartY = nYCnt * 32 + 32 - p->h();
            DrawFunction(p, nStartX, nStartY);
            extern MainWindow *g_MainWindow;
            if(g_MainWindow->ShowGroundObjectLine()){
                fl_rect(nStartX + nDX, nStartY + nDY, p->w(), p->h(), FL_BLUE);
            }
        }
    };

    auto fnCheckFunc = [](uint32_t nFolderIndex, uint32_t nImageIndex, Fl_Shared_Image * &pImage, int nX, int nY){
        extern MainWindow *g_MainWindow;
        // auto p = g_MainWindow->RetrievePNG(nFolderIndex, nImageIndex);
        auto p = g_MainWindow->RetrieveCachedPNG(nFolderIndex, nImageIndex, nX, nY);

        pImage = p;
        return p ? (p->w() == 48 && p->h() == 32) : false;
    };


    extern Mir2Map g_Map;
    int nStartCellX = (std::max)(0, m_OffsetX / 48 - 1);
    int nStartCellY = (std::max)(0, m_OffsetY / 32 - 2);
    int nStopCellX  = (std::min)((m_OffsetX + w()) / 48 + 1, g_Map.Width()  - 1);
    int nStopCellY  = (std::min)((m_OffsetY + h()) / 32 + 2, g_Map.Height() - 1);
    g_Map.DrawObjectTile(nStartCellX, nStartCellY, nStopCellX, nStopCellY, fnCheckFunc, fnDrawObjFunc);
}

void DrawArea::DrawOverGroundObject()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowOverGroundObjectLayer()){ return; }

    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    auto fnDrawObjFunc = [this, nDX, nDY](uint32_t nFolderIndex, uint32_t nImageIndex, Fl_Shared_Image *pImage, int nXCnt, int nYCnt){
        extern MainWindow *g_MainWindow;
        // auto p = g_MainWindow->RetrievePNG(nFolderIndex, nImageIndex);
        auto p = pImage;

        if(!p){
            g_MainWindow->RetrieveCachedPNG(nFolderIndex, nImageIndex, nXCnt, nYCnt);
        }

        if(p){
            // int nStartX = nXCnt * 48 - 200;
            // int nStartY = nYCnt * 32 - 157 + 32 - p->h();
            int nStartX = nXCnt * 48;
            int nStartY = nYCnt * 32 + 32 - p->h();
            DrawFunction(p, nStartX, nStartY);
            extern MainWindow *g_MainWindow;
            if(g_MainWindow->ShowOverGroundObjectLine()){
                fl_rect(nStartX + nDX, nStartY + nDY, p->w(), p->h(), FL_GREEN);
            }
        }
    };

    auto fnCheckFunc = [](uint32_t nFolderIndex, uint32_t nImageIndex, Fl_Shared_Image * &pImage, int nXCnt, int nYCnt){
        extern MainWindow *g_MainWindow;
        auto p = g_MainWindow->RetrieveCachedPNG(nFolderIndex, nImageIndex, nXCnt, nYCnt);
        // auto p = g_MainWindow->RetrievePNG(nFolderIndex, nImageIndex);
        pImage = p;
        return p ? (p->w() != 48 || p->h() != 32) : false;
    };

    extern Mir2Map g_Map;
    int nStartCellX = (std::max)(0, m_OffsetX / 48 - 10);
    int nStartCellY = (std::max)(0, m_OffsetY / 32 - 20);
    int nStopCellX  = (std::min)((m_OffsetX + w()) / 48 + 10, g_Map.Width()  - 1);
    int nStopCellY  = (std::min)((m_OffsetY + h()) / 32 + 20, g_Map.Height() - 1);
    g_Map.DrawObjectTile(nStartCellX, nStartCellY, nStopCellX, nStopCellY, fnCheckFunc, fnDrawObjFunc);
}

void DrawArea::DrawFunction(Fl_Shared_Image *pImage, int nStartX, int nStartY)
{
    // linux needs crop the region, wtf
    // pImage->draw(nStartX + x() - m_OffsetX, nStartY + y() - m_OffsetY);

    int nWX = x();
    int nWY = y();

    int nX = nStartX - m_OffsetX + nWX;
    int nY = nStartY - m_OffsetY + nWY;
    int nW = pImage->w();
    int nH = pImage->h();

    if(pImage == nullptr
            || nX >= nWX + w() || nX + pImage->w() <= nWX
            || nY >= nWY + h() || nY + pImage->h() <= nWY){
        return;
    }

    int nSX = 0;
    int nSY = 0;

    if(nX < nWX){
        nSX += (nWX - nX);
        nW  -= (nWX - nX);
        nX   = nWX;
    }

    if(nX + pImage->w() > nWX + w()){
        nW = nWX + w() - nX;
    }

    if(nY < nWY){
        nSY += (nWY - nY);
        nH  -= (nWY - nY);
        nY   = nWY;
    }

    if(nY + pImage->h() > nWY + h()){
        nH = nWY + h() - nY;
    }

    pImage->draw(nX, nY, nW, nH, nSX, nSY);
}

void DrawArea::DrawFunction(uint32_t nFolderIndex, uint32_t nImageIndex, int nStartX, int nStartY)
{
    extern MainWindow *g_MainWindow;
    DrawFunction(g_MainWindow->RetrievePNG(nFolderIndex, nImageIndex), nStartX, nStartY);
}

void DrawArea::DrawGroundInfo()
{
    //------->   0 
    //-------> 3   1
    //------->   2

    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    auto fnDrawOnNOFunc = [this](int nX, int nY, int nIndex){
        UNUSED(nX);
        UNUSED(nY);
        UNUSED(nIndex);
    };
    auto fnDrawOnYESFunc  = [this, nDX, nDY](int nX, int nY, int nIndex){
        extern MainWindow *g_MainWindow;
        if(g_MainWindow->ShowGroundInfoLine()){
            int nMidX, nMidY, nX1, nY1, nX2, nY2;
            GetTriangleOnMap(nX, nY, nIndex, nMidX, nMidY, nX1, nY1, nX2, nY2);
            fl_loop(nMidX + nDX, nMidY + nDY, nX1 + nDX, nY1 + nDY, nX2 + nDX, nY2 + nDY);
        }
    };

    extern Mir2Map           g_Map;
    extern MainWindow       *g_MainWindow;
    extern GroundInfoWindow *g_GroundInfoWindow;

    extern std::vector<std::vector<std::array<uint32_t, 4>>> g_GroundInfo;

    int nStartCellX = (std::max)(0, m_OffsetX / 48 - 1);
    int nStartCellY = (std::max)(0, m_OffsetY / 32 - 1);
    int nStopCellX  = (std::min)((m_OffsetX + w()) / 48 + 1, g_Map.Width()  - 1);
    int nStopCellY  = (std::min)((m_OffsetY + h()) / 32 + 1, g_Map.Height() - 1);

    std::function<void(int, int, int)> fnOnYes = fnDrawOnYESFunc;
    std::function<void(int, int, int)> fnOnNo  = fnDrawOnNOFunc;
    if(g_MainWindow->ReversedShowGroundInfoLine()){
        std::swap(fnOnYes, fnOnNo);
    }

    {// set color and then draw ground info
        auto wColor = fl_color();
        fl_color(FL_MAGENTA);
        for(int nX = nStartCellX; nX <= nStopCellX; ++nX){
            for(int nY = nStartCellY; nY <= nStopCellY; ++nY){
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(g_GroundInfoWindow->Test(g_GroundInfo[nX][nY][nIndex])){
                        fnOnYes(nX, nY, nIndex);
                    }else{
                        fnOnNo(nX, nY, nIndex);
                    }
                }
            }
        }
        fl_color(wColor);
    }
}

void DrawArea::DrawBaseTile()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowBaseTileLayer()){ return; }

    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    auto fnDrawFunc = [this, nDX, nDY](uint32_t nFolderIndex, uint32_t nImageIndex, int nX, int nY){
        int nStartX = nX * 48;
        int nStartY = nY * 32;
        extern MainWindow *g_MainWindow;
        auto p = g_MainWindow->RetrieveCachedPNG(nFolderIndex, nImageIndex, nX, nY);
        if(p){
            DrawFunction(p, nStartX, nStartY);
            extern MainWindow *g_MainWindow;
            if(g_MainWindow->ShowBaseTileLine()){
                // fl_rect(nStartX + nDX, nStartY + nDY, 96, 64, FL_RED);
                fl_rect(nStartX + nDX, nStartY + nDY, p->w(), p->h(), FL_RED);
            }
        }
    };

    extern Mir2Map g_Map;
    int nStartCellX = (std::max)(0, m_OffsetX / 48 - 1);
    int nStartCellY = (std::max)(0, m_OffsetY / 32 - 1);
    int nStopCellX  = (std::min)((m_OffsetX + w()) / 48 + 1, g_Map.Width()  - 1);
    int nStopCellY  = (std::min)((m_OffsetY + h()) / 32 + 1, g_Map.Height() - 1);
    g_Map.DrawBaseTile(nStartCellX, nStartCellY, nStopCellX, nStopCellY, fnDrawFunc);
}

void DrawArea::SetXOffset(int nX)
{
    m_OffsetX = nX;
}

void DrawArea::SetYOffset(int nY)
{
    m_OffsetY = nY;
}

int DrawArea::handle(int nEvent)
{
    int ret = Fl_Box::handle(nEvent);

    extern Mir2Map g_Map;
    if(!g_Map.Valid()){
        return ret;
    }

    int mouseX = m_MouseX;
    int mouseY = m_MouseY;
    m_MouseX   = Fl::event_x();
    m_MouseY   = Fl::event_y();

    switch(nEvent){
        case FL_RELEASE:
            fl_cursor(FL_CURSOR_DEFAULT);
            break;

        case FL_MOVE:
            break;

        case FL_DRAG:
            if(Fl::event_state() & FL_CTRL){
                // bug of fltk here for windows, when some key is pressed, 
                // event_x() and event_y() are incorrect!
                // printf("%4d %4d\n", Fl::event_x(), Fl::event_y());
                //
                // m_ReferenceLineX += (m_MouseX - mouseX);
                // m_ReferenceLineY += (m_MouseY - mouseY);
            }else{
                m_OffsetX -= (m_MouseX - mouseX);
                m_OffsetY -= (m_MouseY - mouseY);
                m_OffsetX  = (std::max)(m_OffsetX, 0);
                m_OffsetY  = (std::max)(m_OffsetY, 0);
                m_OffsetX  = (std::min)(m_OffsetX, (std::max)(0, 48 * g_Map.Width()  - w()));
                m_OffsetY  = (std::min)(m_OffsetY, (std::max)(0, 32 * g_Map.Height() - h()));


                double fXP = -1.0;
                double fYP = -1.0;
                if(48 * g_Map.Width()  - w() > 0){
                    fXP = m_OffsetX * 1.0 / (48 * g_Map.Width()  - w());
                }
                if(32 * g_Map.Height() - h() > 0){
                    fYP = m_OffsetY * 1.0 / (32 * g_Map.Height() - h());
                }
                extern MainWindow *g_MainWindow;
                g_MainWindow->UpdateScrollBar(fXP, fYP);
            }
            break;

        case FL_PUSH:
            {
                extern MainWindow *g_MainWindow;
                if(g_MainWindow->EnableSelect() && g_MainWindow->SelectByRegion()){
                    extern std::vector<std::pair<int, int>> g_SelectByRegionPointV;
                    g_SelectByRegionPointV.emplace_back(
                            m_MouseX - x() + m_OffsetX, m_MouseY - y() + m_OffsetY);
                }
                if(g_MainWindow->EnableEdit()){
                    if(Fl::event_state() & FL_CTRL){
                        // TODO:
                    }else{
                        SetGroundSubCellUnderPoint(m_MouseX - x() + m_OffsetX, m_MouseY - y() + m_OffsetY);
                    }
                }else{
                    if(Fl::event_state() & FL_CTRL){
                        // TODO:
                    }else{
                        fl_cursor(FL_CURSOR_MOVE);
                    }
                }
            }
            // for drag nEvent
            ret = 1;
            break;
        default:
            break;
    }

    extern MainWindow *g_MainWindow;
    g_MainWindow->RedrawAll();

    return ret;
}

bool DrawArea::LocateGroundSubCell(int nXOnMap, int nYOnMap, int &nX, int &nY, int &nIndex)
{
    extern Mir2Map g_Map;
    if(false
            || nXOnMap < 0
            || nXOnMap >= g_Map.Width() * 48
            || nYOnMap < 0
            || nYOnMap >= g_Map.Height() * 32
      ){
        return false;
    }

    nX = nXOnMap / 48;
    nY = nYOnMap / 32;

    for(int nIndexLoop = 0; nIndexLoop < 4; ++nIndexLoop){
        int nMidX, nMidY, nX1, nY1, nX2, nY2;
        GetTriangleOnMap(nX, nY, nIndexLoop, nMidX, nMidY, nX1, nY1, nX2, nY2);
        if(PointInTriangle(nXOnMap, nYOnMap, nMidX, nMidY, nX1, nY1, nX2, nY2)){
            nIndex = nIndexLoop;
            return true;
        }
    }
    return false;
}

void DrawArea::SetGroundSubCellUnderPoint(int nMouseXOnMap, int nMouseYOnMap)
{
    int nX, nY, nIndex;
    if(LocateGroundSubCell(nMouseXOnMap, nMouseYOnMap, nX, nY, nIndex)){
        // printf("%03d %03d %03d\n", nX, nY, nIndex);
        extern std::vector<std::vector<std::array<uint32_t, 4>>> g_GroundInfo;
        extern GroundInfoWindow *g_GroundInfoWindow;
        // g_GroundInfo[nX][nY][nIndex] |= g_GroundInfoWindow->Mask();
        g_GroundInfo[nX][nY][nIndex] = g_GroundInfoWindow->Mask();
    }
}

void DrawArea::GetTriangleOnMap(
        int nCellX, int nCellY, int nIndex, // ientfy cell
        int &nX0, int &nY0,  // 0
        int &nX1, int &nY1,  // 1
        int &nX2, int &nY2)  // 2
{
    // for triangle index in a grid:
    //------->   0 
    //-------> 3   1
    //------->   2
    //
    //for points in a triangle: use clock-wise:
    //  1
    // /|
    //0 |    1-------2
    // \|     \     /
    //  2      \   /
    //          \ /
    //           0

    // int nStartX = nX * 48 + x() - m_OffsetX;
    // int nStartY = nY * 32 + y() - m_OffsetY;
    int nStartX = nCellX * 48;
    int nStartY = nCellY * 32;
    int nStopX  = nStartX + 48;
    int nStopY  = nStartY + 32;

    nX0 = (nStartX + nStopX) / 2;
    nY0 = (nStartY + nStopY) / 2;

    switch(nIndex % 4){
        case 0:
            nX1 = nStartX;
            nY1 = nStartY;
            nX2 = nStopX;
            nY2 = nStartY;
            break;
        case 1:
            nX1 = nStopX;
            nY1 = nStartY;
            nX2 = nStopX;
            nY2 = nStopY;
            break;
        case 2:
            nX1 = nStopX;
            nY1 = nStopY;
            nX2 = nStartX;
            nY2 = nStopY;
            break;
        case 3:
            nX1 = nStartX;
            nY1 = nStopY;
            nX2 = nStartX;
            nY2 = nStartY;
            break;
        default:
            break;
    }
}

void DrawArea::DrawTriangleUnit(int nCX, int nCY, int nIndex)
{
    static Fl_RGB_Image *pImage[4] = {nullptr, nullptr, nullptr, nullptr};
    if(pImage[nIndex % 4] == nullptr){
        uint32_t nCB = 0X00000000;
        uint32_t nCF = 0X800000FF;
        uint32_t pData[32][48];
        for(int nY = 0; nY < 32; ++nY){
            for(int nX = 0; nX < 48; ++nX){
                switch(nIndex % 4){
                    case 0:
                        if(3 * (16 - nY) >= 2 * std::abs(nX - 24)){
                            pData[nY][nX] = nCF;
                        }else{
                            pData[nY][nX] = nCB;
                        }
                        break;
                    // case 1:
                    //     if(3 * std::abs(nY - 16) <= 2 * (nX - 24)){
                    //         pData[nY][nX] = nCF;
                    //     }else{
                    //         pData[nY][nX] = nCB;
                    //     }
                    //     break;
                    // case 2:
                    //     if(3 * (nY - 16) >= 2 * std::abs(nX - 24)){
                    //         pData[nY][nX] = nCF;
                    //     }else{
                    //         pData[nY][nX] = nCB;
                    //     }
                    //     break;
                    // case 3:
                    //     if(3 * std::abs(nY - 16) <= 2 * (24 - nX)){
                    //         pData[nY][nX] = nCF;
                    //     }else{
                    //         pData[nY][nX] = nCB;
                    //     }
                    //     break;
                    default:
                        pData[nY][nX] = nCB;
                        break;
                }
            }
        }
        pImage[nIndex % 4] = new Fl_RGB_Image((uchar *)(pData), 48, 32, 4);
    }

    if(pImage[nIndex % 4]){
        pImage[nIndex % 4]->draw(nCX * 48 - m_OffsetX + x(), nCY * 32 - m_OffsetY + y());
    }
}

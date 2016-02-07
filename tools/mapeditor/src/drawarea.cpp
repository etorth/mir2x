/*
 * =====================================================================================
 *
 *       Filename: drawarea.cpp
 *        Created: 7/26/2015 4:27:57 AM
 *  Last Modified: 02/07/2016 03:48:42
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

#include "drawarea.hpp"
#include <FL/Fl_Shared_Image.H>
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


DrawArea::DrawArea(int x, int y, int w, int h)
    : Fl_Box(x, y, w, h)
    , m_MouseX(0)
    , m_MouseY(0)
    , m_IsDragging(false)
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
}

void DrawArea::DrawGroundObject()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowGroundObjectLayer()){ return; }

    auto fnDrawObjFunc = [this](uint32_t nFolderIndex, uint32_t nImageIndex, Fl_Shared_Image * pImage, int nXCnt, int nYCnt){
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
                fl_rect(nStartX + x() - m_OffsetX, nStartY + y() - m_OffsetY, p->w(), p->h(), FL_BLUE);
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
    int nStartCellX = (std::max)(0, m_OffsetX / 48 - 10);
    int nStartCellY = (std::max)(0, m_OffsetY / 32 - 20);
    int nStopCellX  = (std::min)((m_OffsetX + w()) / 48 + 10, g_Map.Width()  - 1);
    int nStopCellY  = (std::min)((m_OffsetY + h()) / 32 + 20, g_Map.Height() - 1);
    g_Map.DrawObjectTile(nStartCellX, nStartCellY, nStopCellX, nStopCellY, fnCheckFunc, fnDrawObjFunc);
}

void DrawArea::DrawOverGroundObject()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowOverGroundObjectLayer()){ return; }

    auto fnDrawObjFunc = [this](uint32_t nFolderIndex, uint32_t nImageIndex, Fl_Shared_Image *pImage, int nXCnt, int nYCnt){
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
                fl_rect(nStartX + x() - m_OffsetX, nStartY + y() - m_OffsetY, p->w(), p->h(), FL_GREEN);
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
    if(pImage){
        pImage->draw(nStartX + x() - m_OffsetX, nStartY + y() - m_OffsetY);
    }
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

    auto fnDrawOnNOFunc = [this](int nX, int nY, int nIndex){};
    auto fnDrawOnYESFunc  = [this](int nX, int nY, int nIndex){
        extern MainWindow *g_MainWindow;
        if(g_MainWindow->ShowGroundInfoLine()){
            int nStartX = nX * 48 + x() - m_OffsetX;
            int nStartY = nY * 32 + y() - m_OffsetY;
            int nStopX  = nStartX + 48;
            int nStopY  = nStartY + 32;
            int nMidX   = (nStartX + nStopX) / 2;
            int nMidY   = (nStartY + nStopY) / 2;

            int nE3P1X  = 0;
            int nE3P1Y  = 0;
            int nE3P2X  = 0;
            int nE3P2Y  = 0;
            int nE3P3X  = 0;
            int nE3P3Y  = 0;

            switch(nIndex){
                case 0:
                    nE3P1X = nStartX;
                    nE3P1Y = nStartY;
                    nE3P2X = nStopX;
                    nE3P2Y = nStartY;
                    nE3P3X = nMidX;
                    nE3P3Y = nMidY;
                    break;
                case 1:
                    nE3P1X = nMidX;
                    nE3P1Y = nMidY;
                    nE3P2X = nStopX;
                    nE3P2Y = nStartY;
                    nE3P3X = nStopX;
                    nE3P3Y = nStopY;
                    break;
                case 2:
                    nE3P1X = nStartX;
                    nE3P1Y = nStopY;
                    nE3P2X = nMidX;
                    nE3P2Y = nMidY;
                    nE3P3X = nStopX;
                    nE3P3Y = nStopY;
                    break;
                case 3:
                    nE3P1X = nStartX;
                    nE3P1Y = nStartY;
                    nE3P2X = nMidX;
                    nE3P2Y = nMidY;
                    nE3P3X = nStartX;
                    nE3P3Y = nStopY;
                    break;
                default:
                    break;
            }
            fl_loop(nE3P1X, nE3P1Y, nE3P2X, nE3P2Y, nE3P3X, nE3P3Y);
        }
    };

    extern Mir2Map           g_Map;
    extern MainWindow       *g_MainWindow;
    extern GroundInfoWindow *g_GroundInfoWindow;

    extern std::vector<std::vector<std::array<uint32_t, 4>>> g_GroundInfo;

    int nStartCellX = (std::max)(0, m_OffsetX / 48 - 10);
    int nStartCellY = (std::max)(0, m_OffsetY / 32 - 20);
    int nStopCellX  = (std::min)((m_OffsetX + w()) / 48 + 10, g_Map.Width()  - 1);
    int nStopCellY  = (std::min)((m_OffsetY + h()) / 32 + 20, g_Map.Height() - 1);

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
    auto fnDrawFunc = [this](uint32_t nFolderIndex, uint32_t nImageIndex, int nX, int nY){
        int nStartX = nX * 48;
        int nStartY = nY * 32;
        extern MainWindow *g_MainWindow;
        auto p = g_MainWindow->RetrieveCachedPNG(nFolderIndex, nImageIndex, nX, nY);
        if(p){
            DrawFunction(p, nStartX, nStartY);
            extern MainWindow *g_MainWindow;
            if(g_MainWindow->ShowBaseTileLine()){
                fl_rect(nStartX + x() - m_OffsetX, nStartY + y() - m_OffsetY, 96, 64, FL_RED);
            }
        }
    };

    extern Mir2Map g_Map;
    int nStartCellX = (std::max)(0, m_OffsetX / 48 - 10);
    int nStartCellY = (std::max)(0, m_OffsetY / 32 - 20);
    int nStopCellX  = (std::min)((m_OffsetX + w()) / 48 + 10, g_Map.Width()  - 1);
    int nStopCellY  = (std::min)((m_OffsetY + h()) / 32 + 20, g_Map.Height() - 1);
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
            m_IsDragging = false;
            break;

        case FL_DRAG:
            if(Fl::event_state() & FL_CTRL){
                // bug of fltk here, when some key is pressed, 
                // event_x() and event_y() are incorrect!
                //
                // m_ReferenceLineX += (m_MouseX - mouseX);
                // m_ReferenceLineY += (m_MouseY - mouseY);
            }else{
                if(m_IsDragging){
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
            }
            break;

        case FL_PUSH:
            {
                extern MainWindow *g_MainWindow;
                if(g_MainWindow->EnableEdit()){
                    if(Fl::event_state() & FL_CTRL){
                        // TODO:
                    }else{
                        SetGroundSubCellUnderPoint(m_MouseX - x(), m_MouseY - y());
                    }
                }else{
                    if(Fl::event_state() & FL_CTRL){
                        // TODO:
                    }else{
                        m_IsDragging = true;
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

bool DrawArea::LocateGroundSubCell(int nMouseX, int nMouseY, int &nX, int &nY, int &nIndex)
{
    extern Mir2Map g_Map;
    if(false
            || nMouseX < 0
            || nMouseX >= g_Map.Width() * 48
            || nMouseY < 0
            || nMouseY >= g_Map.Height() * 32){
        return false;
    }

    int nXOnMap = nMouseX + m_OffsetX;
    int nYOnMap = nMouseY + m_OffsetY;

    nX = nXOnMap / 48;
    nY = nYOnMap / 32;

    int nDX = nXOnMap % 48;
    int nDY = nYOnMap % 32;

    int nStartX = 0;
    int nStartY = 0;
    int nStopX  = nStartX + 48;
    int nStopY  = nStartY + 32;
    int nMidX   = (nStartX + nStopX) / 2;
    int nMidY   = (nStartY + nStopY) / 2;

    int nE3P1X  = 0;
    int nE3P1Y  = 0;
    int nE3P2X  = 0;
    int nE3P2Y  = 0;
    int nE3P3X  = 0;
    int nE3P3Y  = 0;

    for(int nIndexLoop = 0; nIndexLoop < 4; ++nIndexLoop){
        switch(nIndexLoop){
            case 0:
                nE3P1X = nStartX;
                nE3P1Y = nStartY;
                nE3P2X = nStopX;
                nE3P2Y = nStartY;
                nE3P3X = nMidX;
                nE3P3Y = nMidY;
                break;
            case 1:
                nE3P1X = nMidX;
                nE3P1Y = nMidY;
                nE3P2X = nStopX;
                nE3P2Y = nStartY;
                nE3P3X = nStopX;
                nE3P3Y = nStopY;
                break;
            case 2:
                nE3P1X = nStartX;
                nE3P1Y = nStopY;
                nE3P2X = nMidX;
                nE3P2Y = nMidY;
                nE3P3X = nStopX;
                nE3P3Y = nStopY;
                break;
            case 3:
                nE3P1X = nStartX;
                nE3P1Y = nStartY;
                nE3P2X = nMidX;
                nE3P2Y = nMidY;
                nE3P3X = nStartX;
                nE3P3Y = nStopY;
                break;
            default:
                break;
        }

        if(CheckInTriangle(nDX, nDY, nE3P1X, nE3P1Y, nE3P2X, nE3P2Y, nE3P3X, nE3P3Y)){
            nIndex = nIndexLoop;
            return true;
        }
    }
    return false;
}

void DrawArea::SetGroundSubCellUnderPoint(int nMouseX, int nMouseY)
{
    int nX, nY, nIndex;
    if(LocateGroundSubCell(nMouseX, nMouseY, nX, nY, nIndex)){
        // printf("%03d %03d %03d\n", nX, nY, nIndex);
        extern std::vector<std::vector<std::array<uint32_t, 4>>> g_GroundInfo;
        extern GroundInfoWindow *g_GroundInfoWindow;
        // g_GroundInfo[nX][nY][nIndex] |= g_GroundInfoWindow->Mask();
        g_GroundInfo[nX][nY][nIndex] = g_GroundInfoWindow->Mask();
    }
}

bool DrawArea::CheckInTriangle(int nX, int nY, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3)
{
    auto bSign = [](double p1X, double p1Y, double p2X, double p2Y, double p3X, double p3Y){
        return (p1X - p3X) * (p2Y - p3Y) - (p2X - p3X) * (p1Y - p3Y);
    };

    bool b1 = bSign((double)nX, (double)nY, (double)nX1, (double)nY1, (double)nX2, (double)nY2) < 0.0f;
    bool b2 = bSign((double)nX, (double)nY, (double)nX2, (double)nY2, (double)nX3, (double)nY3) < 0.0f;
    bool b3 = bSign((double)nX, (double)nY, (double)nX3, (double)nY3, (double)nX1, (double)nY1) < 0.0f;

    return ((b1 == b2) && (b2 == b3));
}

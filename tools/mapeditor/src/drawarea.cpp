/*
 * =====================================================================================
 *
 *       Filename: drawarea.cpp
 *        Created: 7/26/2015 4:27:57 AM
 *  Last Modified: 02/15/2016 21:27:39
 *
 *    Description: To handle or GUI interaction
 *                 Provide handlers to EditorMap
 *                 EditorMap will draw scene with assistance of ImageDB
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

#include <cstring>
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
#include "editormap.hpp"

#include "imagedb.hpp"
#include "imagecache.hpp"

DrawArea::DrawArea(int x, int y, int w, int h)
    : Fl_Box(x, y, w, h)
    , m_MouseX(0)
    , m_MouseY(0)
    , m_OffsetX(0)
    , m_OffsetY(0)
    , m_TUC{nullptr, nullptr, nullptr, nullptr}
    , m_TextBoxBG(nullptr)
{
    m_TUC[0] = CreateTUC(0);
    m_TUC[1] = CreateTUC(1);
    m_TUC[2] = CreateTUC(2);
    m_TUC[3] = CreateTUC(3);
}

DrawArea::~DrawArea()
{
    delete m_TextBoxBG;
    delete m_TUC[0];
    delete m_TUC[1];
    delete m_TUC[2];
    delete m_TUC[3];
}

void DrawArea::draw()
{
    Fl_Box::draw();
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ClearBackground()){
        fl_rectf(x(), y(), w(), h(), 0, 0, 0);
    }

    extern EditorMap g_EditorMap;
    if(!g_EditorMap.Valid()){
        return;
    }

    DrawTile();
    DrawObject(true);
    DrawObject(false);
	DrawGround();

    DrawSelect();
    DrawTrySelect();
    DrawTextBox();
}

void DrawArea::DrawSelectBySingle()
{
    int nX, nY, nIndex;
    if(LocateGroundSubCell( m_MouseX - x() + m_OffsetX,
                m_MouseY - y() + m_OffsetY, nX, nY, nIndex)){
        DrawTUC(nX, nY, nIndex);
    }
}

void DrawArea::AddSelectBySingle()
{
    int nX, nY, nIndex;
    if(LocateGroundSubCell( m_MouseX - x() + m_OffsetX,
                m_MouseY - y() + m_OffsetY, nX, nY, nIndex)){
        extern EditorMap g_EditorMap;
        g_EditorMap.SetGroundSelect(nX, nY, nIndex, 1);
    }
}

void DrawArea::DrawSelectByRegion()
{
    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    auto fnDrawSelect = [nDX, nDY, this](const std::vector<std::pair<int, int>> &stPointV){
        if(stPointV.size() > 1){
            // connect all points
            for(size_t nIndex = 0; nIndex < stPointV.size() - 1; ++nIndex){
                fl_line(stPointV[nIndex].first  + nDX,
                        stPointV[nIndex].second + nDY,
                        stPointV[nIndex + 1].first  + nDX,
                        stPointV[nIndex + 1].second + nDY);
            }
        }

        if(!stPointV.empty()){
            // draw all points as small circles
            for(auto &stPair: stPointV){
                fl_circle(stPair.first + nDX, stPair.second +nDY, 4.0);
            }
            // draw current moving line
            fl_line(m_MouseX, m_MouseY,
                    stPointV.back().first + nDX,
                    stPointV.back().second + nDY);
        }
    };

    extern EditorMap g_EditorMap;
    g_EditorMap.DrawSelectPoint(fnDrawSelect);
}

void DrawArea::RhombusCoverOperation(int nMX, int nMY, int nSize,
        std::function<void(int, int, int)> fnOperation)
{
    if(nSize <= 0){ return; }

    // don't check boundary condition
    // since even center point is out of map
    // we can still select grids over map

    int nCX = nMX / 48;
    int nCY = nMY / 32 - nSize / 2;

    // mode 0: 0, 2
    // mode 1: 1, 3

    int nStartMode = (2 * (nMX % 48) < 3 * (nMY % 32) ) ? 0 : 1;
    int nMode = nStartMode;
    int nLine = 1;

    auto fnLineCoverInfo = [nCX, nCY, nStartMode](
            int &nStartX, int &nStartY, int &nCnt, int nLine){
        nStartX = nCX - (nLine - nStartMode) / 2;
        nStartY = nCY + (nLine - nStartMode) / 2;
        nCnt    = nLine;
    };

    while(nLine < nSize * 2){
        int nStartX, nStartY, nCnt;
        if(nLine <= nSize){
            fnLineCoverInfo(nStartX, nStartY, nCnt, nLine);
        }else{
            fnLineCoverInfo(nStartX, nStartY, nCnt, nSize * 2 - nLine);
            nStartY += (nLine - nSize);
        }

        for(int nIndex = 0; nIndex < nCnt; ++nIndex){
            if(nMode == 0){
                fnOperation(nStartX + nIndex, nStartY    , 2);
                fnOperation(nStartX + nIndex, nStartY + 1, 0);
            }else{
                fnOperation(nStartX + nIndex    , nStartY, 1);
                fnOperation(nStartX + nIndex + 1, nStartY, 3);
            }
        }
        nMode = 1 - nMode;
        nLine++;
    }
}

void DrawArea::DrawSelectByRhombus()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->RhombusSize();
    int nMX   = m_MouseX + m_OffsetX - x();
    int nMY   = m_MouseY + m_OffsetY - y();

    auto fnDraw = [this](int nX,  int nY, int nIndex){
        DrawTUC(nX, nY, nIndex);
    };

    RhombusCoverOperation(nMX, nMY, nSize, fnDraw);
}

void DrawArea::AddSelectByRhombus()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->RhombusSize();
    int nMX   = m_MouseX + m_OffsetX - x();
    int nMY   = m_MouseY + m_OffsetY - y();

    auto fnSet = [](int nX,  int nY, int nIndex){
        extern EditorMap g_EditorMap;
        g_EditorMap.SetGroundSelect(nX, nY, nIndex, 1);
    };

    RhombusCoverOperation(nMX, nMY, nSize, fnSet);
}

void DrawArea::RectangleCoverOperation(
        int nMouseXOnMap, int nMouseYOnMap, int nSize, std::function<void(int, int, int)> fnOperation)
{
    if(nSize <= 0){ return; }

    int nMX = nMouseXOnMap / 48 - nSize / 2;
    int nMY = nMouseYOnMap / 32 - nSize / 2;

    for(int nX = 0; nX < nSize; ++nX){
        for(int nY = 0; nY < nSize; ++nY){
            for(int nIndex = 0; nIndex < 4; ++nIndex){
                fnOperation(nX + nMX, nY + nMY, nIndex);
            }
        }
    }
}

void DrawArea::DrawSelectByRectangle()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->RectangleSize();
    int nMX   = m_MouseX + m_OffsetX - x();
    int nMY   = m_MouseY + m_OffsetY - y();

    auto fnDraw = [this](int nX,  int nY, int nIndex){
        DrawTUC(nX, nY, nIndex);
    };

    RectangleCoverOperation(nMX, nMY, nSize, fnDraw);
}

void DrawArea::AddSelectByRectangle()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->RectangleSize();
    int nMX   = m_MouseX + m_OffsetX - x();
    int nMY   = m_MouseY + m_OffsetY - y();

    auto fnSet = [](int nX,  int nY, int nIndex){
        extern EditorMap g_EditorMap;
        g_EditorMap.SetGroundSelect(nX, nY, nIndex, 1);
    };

    RectangleCoverOperation(nMX, nMY, nSize, fnSet);
}

void DrawArea::DrawSelect()
{
    extern EditorMap g_EditorMap;

    int nX = m_OffsetX / 48 - 1;
    int nY = m_OffsetY / 32 - 1;

    int nXSize = w() / 48 + 3;
    int nYSize = h() / 32 + 3;

    for(int nTX = nX; nTX < nXSize + nX; ++nTX){
        for(int nTY = nY; nTY < nYSize + nY; ++nTY){
            if(g_EditorMap.ValidC(nTX, nTY)){
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(g_EditorMap.GroundSelect(nTX, nTY, nIndex)){
                        DrawTUC(nTX, nTY, nIndex);
                    }
                }
            }
        }
    }
}

void DrawArea::DrawTrySelect()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->EnableSelect()){
        auto wColor = fl_color();
        fl_color(FL_RED);

        if(g_MainWindow->SelectBySingle()){
            DrawSelectBySingle();
        }

        if(g_MainWindow->SelectByRegion()){
            DrawSelectByRegion();
        }

        if(g_MainWindow->SelectByRhombus()){
            DrawSelectByRhombus();
        }

        if(g_MainWindow->SelectByRectangle()){
            DrawSelectByRectangle();
        }

        fl_color(wColor);
    }
}

void DrawArea::DrawTextBox()
{
    if(m_TextBoxBG == nullptr){
        uint32_t pData[32 * 5][48 * 4];
        for(int nY = 0; nY < 32 * 5; ++nY){
            for(int nX = 0; nX < 48 * 4; ++nX){
                pData[nY][nX] = 0X80000000;
            }
        }
        m_TextBoxBG = Fl_RGB_Image((uchar *)pData, 48 * 4, 32 * 5, 4, 0).copy(48 * 4, 32 * 5);
    }

    DrawFunction(m_TextBoxBG, m_OffsetX, m_OffsetY);

    auto wColor = fl_color();
    fl_color(FL_RED);

    char szInfo[128];
    std::sprintf(szInfo, "OffsetX: %d %d", m_OffsetX / 48, m_OffsetX);
    fl_draw(szInfo, 10 + x(), 20 + y());

    std::sprintf(szInfo, "OffsetY: %d %d", m_OffsetY / 32, m_OffsetY);
    fl_draw(szInfo, 10 + x(), 40 + y());

    int nMX = std::max(0, m_MouseX + m_OffsetX - x());
    int nMY = std::max(0, m_MouseY + m_OffsetY - y());

    std::sprintf(szInfo, "MouseMX: %d %d", nMX / 48, nMX);
    fl_draw(szInfo, 10 + x(), 60 + y());

    std::sprintf(szInfo, "MouseMY: %d %d", nMY / 32, nMY);
    fl_draw(szInfo, 10 + x(), 80 + y());

    fl_color(wColor);
}

void DrawArea::DrawObject(bool bGround)
{
    extern MainWindow *g_MainWindow;
    if(bGround){
        if(!g_MainWindow->ShowGroundObjectLayer()){ return; }
    }else{
        if(!g_MainWindow->ShowOverGroundObjectLayer()){ return; }
    }

    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    auto fnDrawObj = [this, nDX, nDY, bGround](uint8_t nFileIndex, uint16_t nImageIndex, int nXCnt, int nYCnt){
        auto p = RetrievePNG(nFileIndex, nImageIndex);
        if(p){
            // int nStartX = nXCnt * 48 - 200;
            // int nStartY = nYCnt * 32 - 157 + 32 - p->h();
            int nStartX = nXCnt * 48;
            int nStartY = nYCnt * 32 + 32 - p->h();
            DrawFunction(p, nStartX, nStartY);
            extern MainWindow *g_MainWindow;
            if(bGround){
                if(g_MainWindow->ShowGroundObjectLine()){
                    fl_rect(nStartX + nDX, nStartY + nDY, p->w(), p->h(), FL_BLUE);
                }
            }else{
                if(g_MainWindow->ShowOverGroundObjectLine()){
                    fl_rect(nStartX + nDX, nStartY + nDY, p->w(), p->h(), FL_GREEN);
                }
            }
        }
    };

    extern EditorMap g_EditorMap;
    g_EditorMap.DrawObject(
            m_OffsetX / 48 - 10, m_OffsetY / 32 - 20, w() / 48 + 20, h() / 32 + 40, bGround, fnDrawObj);
}

void DrawArea::DrawFunction(Fl_Image *pImage, int nStartX, int nStartY)
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

void DrawArea::DrawGround()
{
    //------->   0 
    //-------> 3   1
    //------->   2

    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    auto fnDrawOnNO = [this](int nX, int nY, int nIndex){
        UNUSED(nX);
        UNUSED(nY);
        UNUSED(nIndex);
    };
    auto fnDrawOnYES  = [this, nDX, nDY](int nX, int nY, int nIndex){
        extern MainWindow *g_MainWindow;
        if(g_MainWindow->ShowGroundInfoLine()){
            int nMidX, nMidY, nX1, nY1, nX2, nY2;
            GetTriangleOnMap(nX, nY, nIndex, nMidX, nMidY, nX1, nY1, nX2, nY2);
            fl_loop(nMidX + nDX, nMidY + nDY, nX1 + nDX, nY1 + nDY, nX2 + nDX, nY2 + nDY);
        }
    };

    extern EditorMap         g_EditorMap;
    extern MainWindow       *g_MainWindow;
    extern GroundInfoWindow *g_GroundInfoWindow;

    int nStartCellX = (std::max)(0, m_OffsetX / 48 - 1);
    int nStartCellY = (std::max)(0, m_OffsetY / 32 - 1);
    int nStopCellX  = (std::min)((m_OffsetX + w()) / 48 + 1, g_EditorMap.W() - 1);
    int nStopCellY  = (std::min)((m_OffsetY + h()) / 32 + 1, g_EditorMap.H() - 1);

    std::function<void(int, int, int)> fnOnYes = fnDrawOnYES;
    std::function<void(int, int, int)> fnOnNo  = fnDrawOnNO;
    if(g_MainWindow->ReversedShowGroundInfoLine()){
        std::swap(fnOnYes, fnOnNo);
    }

    {// set color and then draw ground info
        auto wColor = fl_color();
        fl_color(FL_MAGENTA);
        for(int nX = nStartCellX; nX <= nStopCellX; ++nX){
            for(int nY = nStartCellY; nY <= nStopCellY; ++nY){
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(g_EditorMap.GroundValid(nX, nY, nIndex)){
                        if(g_GroundInfoWindow->Test((uint32_t)(g_EditorMap.Ground(nX, nY, nIndex)))){
                            fnOnYes(nX, nY, nIndex);
                        }else{
                            fnOnNo(nX, nY, nIndex);
                        }
                    }
                }
            }
        }
        fl_color(wColor);
    }
}

Fl_Image *DrawArea::RetrievePNG(uint8_t nFileIndex, uint16_t nImageIndex)
{
    extern ImageDB    g_ImageDB;
    extern ImageCache g_ImageCache;
    auto p = g_ImageCache.Retrieve(nFileIndex, nImageIndex);
    if(p == nullptr){
        if(g_ImageDB.Valid(nFileIndex, nImageIndex)){
            int nW = g_ImageDB.FastW(nFileIndex);
            int nH = g_ImageDB.FastH(nFileIndex);
            g_ImageCache.Register(nFileIndex, nImageIndex, 
                    g_ImageDB.FastDecode(nFileIndex, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF), nW, nH);
            p = g_ImageCache.Retrieve(nFileIndex, nImageIndex);
        }
    }
    return p;
}

void DrawArea::DrawTile()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowBaseTileLayer()){ return; }

    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    auto fnDraw = [this, nDX, nDY](uint8_t nFileIndex, uint16_t nImageIndex, int nX, int nY){
        int nStartX = nX * 48;
        int nStartY = nY * 32;
        auto p = RetrievePNG(nFileIndex, nImageIndex);
        if(p){
            DrawFunction(p, nStartX, nStartY);
            extern MainWindow *g_MainWindow;
            if(g_MainWindow->ShowBaseTileLine()){
                // fl_rect(nStartX + nDX, nStartY + nDY, 96, 64, FL_RED);
                fl_rect(nStartX + nDX, nStartY + nDY, p->w(), p->h(), FL_RED);
            }
        }
    };

    extern EditorMap g_EditorMap;
    g_EditorMap.DrawTile(
            m_OffsetX / 48 - 5, m_OffsetY / 32 - 5, w() / 48 + 10, h() / 32 + 10, fnDraw);
}

void DrawArea::SetOffset(int nX, bool bRelativeX, int nY, bool bRelativeY)
{
    extern EditorMap g_EditorMap;
    if(!g_EditorMap.Valid()){ return; }

    if(bRelativeX){
        m_OffsetX += nX;
    }else{
        m_OffsetX = nX;
    }
    m_OffsetX = (std::max)(m_OffsetX, 0);
    m_OffsetX = (std::min)(m_OffsetX, (std::max)(0, 48 * g_EditorMap.W() - w()));

    if(bRelativeY){
        m_OffsetY += nY;
    }else{
        m_OffsetY = nY;
    }
    m_OffsetY  = (std::max)(m_OffsetY, 0);
    m_OffsetY  = (std::min)(m_OffsetY, (std::max)(0, 32 * g_EditorMap.H() - h()));
}

int DrawArea::handle(int nEvent)
{
    int ret = Fl_Box::handle(nEvent);

    extern EditorMap g_EditorMap;
    if(!g_EditorMap.Valid()){
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
            {
                extern MainWindow *g_MainWindow;
                if(g_MainWindow->EnableSelect()){
                    AddSelect();
                }else if(g_MainWindow->EnableEdit()){
                    // TODO
                    //
                }else{
                    if(Fl::event_state() & FL_CTRL){
                        // bug of fltk here for windows, when some key is pressed, 
                        // event_x() and event_y() are incorrect!
                    }else{
                        SetOffset(-(m_MouseX - mouseX), true, -(m_MouseY - mouseY), true);
                        SetScrollBar();
                    }
                }
            }
            break;

        case FL_PUSH:
            {
                extern MainWindow *g_MainWindow;
                if(g_MainWindow->EnableSelect()){
                    AddSelect();
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
    extern EditorMap g_EditorMap;
    if(!g_EditorMap.ValidP(nXOnMap, nYOnMap)){ return false; }

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
        extern GroundInfoWindow *g_GroundInfoWindow;
        extern EditorMap g_EditorMap;
        g_EditorMap.SetGround(nX, nY, nIndex, true, (uint8_t)g_GroundInfoWindow->Mask());
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

Fl_Image *DrawArea::CreateTUC(int nIndex)
{
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
                case 1:
                    if(3 * std::abs(nY - 16) <= 2 * (nX - 24)){
                        pData[nY][nX] = nCF;
                    }else{
                        pData[nY][nX] = nCB;
                    }
                    break;
                case 2:
                    if(3 * (nY - 16) >= 2 * std::abs(nX - 24)){
                        pData[nY][nX] = nCF;
                    }else{
                        pData[nY][nX] = nCB;
                    }
                    break;
                case 3:
                    if(3 * std::abs(nY - 16) <= 2 * (24 - nX)){
                        pData[nY][nX] = nCF;
                    }else{
                        pData[nY][nX] = nCB;
                    }
                    break;
                default:
                    pData[nY][nX] = nCB;
                    break;
            }
        }
    }
    return Fl_RGB_Image((uchar *)(pData), 48, 32, 4, 0).copy(48, 32);
}

void DrawArea::DrawTUC(int nCX, int nCY, int nIndex)
{
    extern EditorMap g_EditorMap;
    if(nCX >= 0 && nCX < g_EditorMap.W() && nCY >= 0 && nCY < g_EditorMap.H()){
        DrawFunction(m_TUC[nIndex % 4], nCX * 48, nCY * 32);
    }
}

void DrawArea::AddSelectByRegion()
{
    int nMX = m_MouseX - x() + m_OffsetX;
    int nMY = m_MouseY - y() + m_OffsetY;

    extern EditorMap g_EditorMap;
    g_EditorMap.AddSelectPoint(nMX, nMY);
}

void DrawArea::ClearGroundSelect()
{
    extern EditorMap g_EditorMap;
    if(!g_EditorMap.Valid()){
        return;
    }

    g_EditorMap.ClearGroundSelect();
}

void DrawArea::SetScrollBar()
{
    extern EditorMap g_EditorMap;
    double fXP = -1.0;
    double fYP = -1.0;
    if(48 * g_EditorMap.W()  - w() > 0){
        fXP = m_OffsetX * 1.0 / (48 * g_EditorMap.W()  - w());
    }
    if(32 * g_EditorMap.H() - h() > 0){
        fYP = m_OffsetY * 1.0 / (32 * g_EditorMap.H() - h());
    }

    extern MainWindow *g_MainWindow;
    g_MainWindow->UpdateScrollBar(fXP, fYP);
}

void DrawArea::AddSelect()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->SelectByRegion()){
        AddSelectByRegion();
    }

    if(g_MainWindow->SelectBySingle()){
        AddSelectBySingle();
    }

    if(g_MainWindow->SelectByRhombus()){
        AddSelectByRhombus();
    }

    if(g_MainWindow->SelectByRectangle()){
        AddSelectByRectangle();
    }
}

/*
 * =====================================================================================
 *
 *       Filename: drawarea.cpp
 *        Created: 7/26/2015 4:27:57 AM
 *  Last Modified: 08/08/2016 00:20:46
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
#include "attributeselectwindow.hpp"
#include "animationselectwindow.hpp"
#include "mathfunc.hpp"
#include "editormap.hpp"
#include "animation.hpp"
#include "animationdb.hpp"
#include "animationdraw.hpp"
#include "sysconst.hpp"

#include "imagedb.hpp"
#include "imagecache.hpp"

DrawArea::DrawArea(int x, int y, int w, int h)
    : Fl_Box(x, y, w, h)
    , m_MouseX(0)
    , m_MouseY(0)
    , m_OffsetX(0)
    , m_OffsetY(0)
    , m_TUC{{nullptr, nullptr, nullptr, nullptr}, {nullptr, nullptr, nullptr, nullptr}}
    , m_TextBoxBG(nullptr)
    , m_LightUC(nullptr)
    , m_CoverV(SYS_MAXR + 1, nullptr)
{
    m_TUC[0][0] = CreateTUC(0, true);
    m_TUC[0][1] = CreateTUC(1, true);
    m_TUC[0][2] = CreateTUC(2, true);
    m_TUC[0][3] = CreateTUC(3, true);

    m_TUC[1][0] = CreateTUC(0, false);
    m_TUC[1][1] = CreateTUC(1, false);
    m_TUC[1][2] = CreateTUC(2, false);
    m_TUC[1][3] = CreateTUC(3, false);

    for(int nR = 0; nR < (int)m_CoverV.size(); ++nR){
        m_CoverV[nR] = CreateCover(nR);
    }
}

DrawArea::~DrawArea()
{
    delete m_LightUC;
    delete m_TextBoxBG;
    delete m_TUC[0][0];
    delete m_TUC[0][1];
    delete m_TUC[0][2];
    delete m_TUC[0][3];
    delete m_TUC[1][0];
    delete m_TUC[1][1];
    delete m_TUC[1][2];
    delete m_TUC[1][3];

    for(int nR = 0; nR < (int)m_CoverV.size(); ++nR){
        delete m_CoverV[nR];
    }
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

    DrawAttributeGrid();

    DrawObject(true);
    DrawObject(false);

    DrawLight();

    DrawGrid();

    DrawSelect();
    DrawTrySelect();
    DrawTextBox();
}

void DrawArea::DrawSelectBySingle()
{
    int nX, nY, nIndex;
    if(LocateGroundSubCell( m_MouseX - x() + m_OffsetX,
                m_MouseY - y() + m_OffsetY, nX, nY, nIndex)){
        extern MainWindow *g_MainWindow;
        DrawTUC(nX, nY, nIndex, !g_MainWindow->Deselect());
    }
}

void DrawArea::AddSelectBySingle()
{
    int nX, nY, nIndex;
    if(LocateGroundSubCell( m_MouseX - x() + m_OffsetX,
                m_MouseY - y() + m_OffsetY, nX, nY, nIndex)){
        extern EditorMap g_EditorMap;
        extern MainWindow *g_MainWindow;
        g_EditorMap.SetGroundSelect(nX, nY, nIndex, g_MainWindow->Deselect() ? 0 : 1);
    }
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
        extern MainWindow *g_MainWindow;
        DrawTUC(nX, nY, nIndex, !g_MainWindow->Deselect());
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
        extern MainWindow *g_MainWindow;
        g_EditorMap.SetGroundSelect(nX, nY, nIndex, g_MainWindow->Deselect() ? 0 : 1);
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
        extern MainWindow *g_MainWindow;
        DrawTUC(nX, nY, nIndex, !g_MainWindow->Deselect());
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
        extern MainWindow *g_MainWindow;
        g_EditorMap.SetGroundSelect(nX, nY, nIndex, g_MainWindow->Deselect() ? 0 : 1);
    };

    RectangleCoverOperation(nMX, nMY, nSize, fnSet);
}

void DrawArea::DrawSelectByAttribute()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->AttributeSize();

    if(nSize <= 0){ return; }

    int nMX = m_MouseX + m_OffsetX - x();
    int nMY = m_MouseY + m_OffsetY - y();

    auto fnDraw = [this](int nX,  int nY, int nIndex){
        extern AttributeSelectWindow *g_AttributeSelectWindow;
        extern MainWindow *g_MainWindow;
        extern EditorMap g_EditorMap;
        bool bValid = g_EditorMap.GroundValid(nX, nY, nIndex);
        uint8_t nValue = (bValid ? g_EditorMap.Ground(nX, nY, nIndex) : 0);
        if(g_AttributeSelectWindow->Test(bValid, nValue)){
            DrawTUC(nX, nY, nIndex, !g_MainWindow->Deselect());
        }
    };

    AttributeCoverOperation(nMX, nMY, nSize, fnDraw);

    extern EditorMap g_EditorMap;
    int nAMaxX = std::min(w(), g_EditorMap.W() * 48 - m_OffsetX);
    int nAMaxY = std::min(h(), g_EditorMap.H() * 32 - m_OffsetY);

    int nAX = (nMX / 48 - (nSize / 2)) * 48 - m_OffsetX;
    int nAY = (nMY / 32 - (nSize / 2)) * 32 - m_OffsetY;
    int nAW = nSize * 48;
    int nAH = nSize * 32;

    nAX = std::min(nAX, nAMaxX);
    nAY = std::min(nAY, nAMaxY);

    if(nAX < 0){
        nAW += nAX;
        nAX  = 0;
    }

    if(nAY < 0){
        nAH += nAY;
        nAY  = 0;
    }

    if(nAX + nAW > nAMaxX){
        nAW = nAMaxX - nAX;
    }

    if(nAY + nAH > nAMaxY){
        nAH = nAMaxY - nAY;
    }

    auto wColor = fl_color();
    fl_color(FL_YELLOW);
    DrawRectangle(nAX, nAY, nAW, nAH);
    fl_color(wColor);
}

void DrawArea::AddSelectByAttribute()
{
    extern SelectSettingWindow *g_SelectSettingWindow;

    int nSize = g_SelectSettingWindow->AttributeSize();
    int nMX   = m_MouseX + m_OffsetX - x();
    int nMY   = m_MouseY + m_OffsetY - y();

    auto fnSet = [](int nX,  int nY, int nIndex){
        extern MainWindow *g_MainWindow;
        extern EditorMap g_EditorMap;
        bool bValid = g_EditorMap.GroundValid(nX, nY, nIndex);
        uint8_t nValue = (bValid ? g_EditorMap.Ground(nX, nY, nIndex) : 0);

        // TODO think about it
        extern AttributeSelectWindow *g_AttributeSelectWindow;
        if(g_AttributeSelectWindow->Test(bValid, nValue)){
            g_EditorMap.SetGroundSelect(nX, nY, nIndex, g_MainWindow->Deselect() ? 0 : 1);
        }
    };
    AttributeCoverOperation(nMX, nMY, nSize, fnSet);
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
                    extern MainWindow *g_MainWindow;
                    if(g_EditorMap.GroundSelect(nTX, nTY, nIndex) == !g_MainWindow->Reversed()){
                        DrawTUC(nTX, nTY, nIndex, !g_MainWindow->Reversed());
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

        if(g_MainWindow->SelectByRhombus()){
            DrawSelectByRhombus();
        }

        if(g_MainWindow->SelectByRectangle()){
            DrawSelectByRectangle();
        }

        if(g_MainWindow->SelectByAttribute()){
            DrawSelectByAttribute();
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

    DrawImage(m_TextBoxBG, 0, 0);

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
    auto wColor = fl_color();
    if(bGround){
        if(!g_MainWindow->ShowGroundObjectLayer()){ return; }
        fl_color(FL_BLUE);
    }else{
        if(!g_MainWindow->ShowOverGroundObjectLayer()){ return; }
        fl_color(FL_GREEN);
    }

    auto fnDrawExt = [this, bGround](int nXCnt, int nYCnt){
        if(!bGround){
            extern MainWindow *g_MainWindow;
            if(g_MainWindow->EnableTest()){
                extern AnimationDB g_AnimationDB;
                extern AnimationDraw g_AnimationDraw;

                if(g_AnimationDraw.MonsterID){
                    auto &rstAnimation = g_AnimationDB.RetrieveAnimation(g_AnimationDraw.MonsterID);
                    if(g_AnimationDraw.X / 48 == nXCnt && g_AnimationDraw.Y / 32 == nYCnt){
                        if(rstAnimation.ResetFrame(g_AnimationDraw.Action, g_AnimationDraw.Direction, g_AnimationDraw.Frame)){
                            auto fnDraw = [this](Fl_Shared_Image *pPNG, int nMapX, int nMapY){
                                DrawImage(pPNG, nMapX - m_OffsetX, nMapY - m_OffsetY);
                            };

                            // use R from the AnimationSelectWindow, rather than the copy in AnimationDraw
                            // this enables me to adjust R without reset the animation MonsterID
                            extern AnimationSelectWindow *g_AnimationSelectWindow;
                            int nAnimationX = g_AnimationDraw.X - m_OffsetX;
                            int nAnimationY = g_AnimationDraw.Y - m_OffsetY;
                            int nAnimationR = g_AnimationSelectWindow->R();

                            // draw funcitons take coordinates w.r.t the window rather than the widget
                            // fl_circle(x() + nAnimationX * 1.0, y() + nAnimationY * 1.0, nAnimationR * 1.0);
                            DrawImage(m_CoverV[nAnimationR], nAnimationX - nAnimationR, nAnimationY - nAnimationR);
                            fl_circle(x() + nAnimationX * 1.0, y() + nAnimationY * 1.0, nAnimationR * 1.0);
                            rstAnimation.Draw(g_AnimationDraw.X, g_AnimationDraw.Y, fnDraw);
                        }
                    }
                }
            }
        }
    };

    auto fnDrawObj = [this, bGround](uint8_t nFileIndex, uint16_t nImageIndex, int nXCnt, int nYCnt){
        auto p = RetrievePNG(nFileIndex, nImageIndex);
        if(p){
            // TODO: till now I still have no idea of this offset (-200, -157), I think maybe it's
            //       nothing and I can just ignore it
            //
            // int nStartX = nXCnt * 48 - 200;
            // int nStartY = nYCnt * 32 - 157 + 32 - p->h();
            int nStartX = nXCnt * 48 - m_OffsetX;
            int nStartY = nYCnt * 32 + 32 - p->h() - m_OffsetY;
            DrawImage(p, nStartX, nStartY);
            extern MainWindow *g_MainWindow;
            if(bGround){
                if(g_MainWindow->ShowGroundObjectLine()){
                    DrawRectangle(nStartX, nStartY, p->w(), p->h());
                }
            }else{
                // ok we're drawing over-ground object
                if(g_MainWindow->ShowOverGroundObjectLine()){
                    DrawRectangle(nStartX, nStartY, p->w(), p->h());
                }
            }
        }
    };

    extern EditorMap g_EditorMap;
    g_EditorMap.DrawObject(m_OffsetX / 48 - 10, m_OffsetY / 32 - 20, w() / 48 + 20, h() / 32 + 40, bGround, fnDrawObj, fnDrawExt);

    fl_color(wColor);
}

// coordinate (0, 0) is on most top-left of *DrawArea*
// not for map or window
void DrawArea::DrawImage(Fl_Image *pImage, int nAX, int nAY)
{
    if(pImage == nullptr
            || nAX >= w() || nAX + pImage->w() <= 0
            || nAY >= h() || nAY + pImage->h() <= 0){ return; }

    int nX = nAX + x();
    int nY = nAY + y();
    int nW = pImage->w();
    int nH = pImage->h();

    int nSX = 0;
    int nSY = 0;

    if(nAX < 0){
        nSX -= nAX;
        nW  += nAX;
        nX   = x();
    }

    if(nAX + pImage->w() > w()){
        nW = w() - nAX;
    }

    if(nAY < 0){
        nSY -= nAY;
        nH  += nAY;
        nY   = y();
    }

    if(nAY + pImage->h() > h()){
        nH = h() - nAY;
    }

    pImage->draw(nX, nY, nW, nH, nSX, nSY);
}

void DrawArea::DrawAttributeGrid()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowAttributeGridLine()){ return; }

    auto wColor = fl_color();
    fl_color(FL_MAGENTA);

    for(int nCX = m_OffsetX / 48 - 1; nCX < (m_OffsetX + w()) / 48 + 1; ++nCX){
        for(int nCY = m_OffsetY / 32 - 1; nCY < (m_OffsetY + h()) / 32 + 1; ++nCY){
            for(int nIndex = 0; nIndex < 4; ++nIndex){
                extern EditorMap g_EditorMap;
                if(g_EditorMap.ValidC(nCX, nCY)){
                    bool    bValid = (g_EditorMap.GroundValid(nCX, nCY, nIndex) != 0);
                    uint8_t nValue = (g_EditorMap.Ground(nCX, nCY, nIndex));

                    extern AttributeGridWindow *g_AttributeGridWindow;
                    if(g_AttributeGridWindow->Test(bValid, nValue)){
                        int nMidX, nMidY, nX1, nY1, nX2, nY2;
                        GetTriangleOnMap(nCX, nCY, nIndex, nMidX, nMidY, nX1, nY1, nX2, nY2);
                        // TODO
                        DrawLoop(nMidX - m_OffsetX, nMidY - m_OffsetY, nX1 - m_OffsetX, nY1 - m_OffsetY, nX2 - m_OffsetX, nY2 - m_OffsetY);
                    }
                }
            }
        }
    }

    fl_color(wColor);
}

void DrawArea::DrawGrid()
{
    // this function draw all grids for assistance
    // for selection purpose, use selection operation
    //------->   0 
    //-------> 3   1
    //------->   2

    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowGridLine()){ return; }

    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    auto wColor = fl_color();
    fl_color(FL_MAGENTA);

    for(int nCX = m_OffsetX / 48 - 1; nCX < (m_OffsetX + w()) / 48 + 1; ++nCX){
        for(int nCY = m_OffsetY / 32 - 1; nCY < (m_OffsetY + h()) / 32 + 1; ++nCY){
            for(int nIndex = 0; nIndex < 4; ++nIndex){
                extern EditorMap g_EditorMap;
                if(g_EditorMap.ValidC(nCX, nCY)){
                    int nMidX, nMidY, nX1, nY1, nX2, nY2;
                    GetTriangleOnMap(nCX, nCY, nIndex, nMidX, nMidY, nX1, nY1, nX2, nY2);
                    // TODO
                    fl_loop(nMidX + nDX, nMidY + nDY, nX1 + nDX, nY1 + nDY, nX2 + nDX, nY2 + nDY);
                }
            }
        }
    }

    fl_color(wColor);
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

    auto wColor = fl_color();
    fl_color(FL_RED);

    auto fnDraw = [this](uint8_t nFileIndex, uint16_t nImageIndex, int nX, int nY){
        int nStartX = nX * 48 - m_OffsetX;
        int nStartY = nY * 32 - m_OffsetY;
        auto p = RetrievePNG(nFileIndex, nImageIndex);
        if(p){
            DrawImage(p, nStartX, nStartY);
            extern MainWindow *g_MainWindow;
            if(g_MainWindow->ShowBaseTileLine()){
                DrawRectangle(nStartX, nStartY, p->w(), p->h());
            }
        }
    };

    extern EditorMap g_EditorMap;
    g_EditorMap.DrawTile(
            m_OffsetX / 48 - 5, m_OffsetY / 32 - 5, w() / 48 + 10, h() / 32 + 10, fnDraw);

    fl_color(wColor);
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
                }else if(g_MainWindow->EnableTest()){
                    // we are moving the animation to a proper place
                    // if current position is invalid, then we permit any moving to get a valid
                    // position, but if current position is valid, we reject any move request
                    // which make the position invlaid again

                    extern AnimationDraw g_AnimationDraw;
                    extern AnimationSelectWindow *g_AnimationSelectWindow;
                    int nNewX = g_AnimationDraw.X + (m_MouseX - mouseX);
                    int nNewY = g_AnimationDraw.Y + (m_MouseY - mouseY);

                    if(CoverValid(g_AnimationDraw.X, g_AnimationDraw.Y, g_AnimationSelectWindow->R())){
                        if(CoverValid(nNewX, nNewY, g_AnimationSelectWindow->R())){
                            g_AnimationDraw.X = nNewX;
                            g_AnimationDraw.Y = nNewY;
                        }else{
                            // try to find a feasible internal point by binary search
                            int nX0 = g_AnimationDraw.X;
                            int nY0 = g_AnimationDraw.Y;
                            int nX1 = nNewX;
                            int nY1 = nNewY;
                            while((std::abs(nX1 - nX0) >= 2) && (std::abs(nY1 - nY0) >= 2)){
                                int nMidX = (nX0 + nX1) / 2;
                                int nMidY = (nY0 + nY1) / 2;

                                if(CoverValid(nMidX, nMidY, g_AnimationSelectWindow->R())){
                                    nX0 = nMidX;
                                    nY0 = nMidY;
                                }else{
                                    nX1 = nMidX;
                                    nY1 = nMidY;
                                }
                            }
                            g_AnimationDraw.X = nX0;
                            g_AnimationDraw.Y = nY0;
                        }
                    }else{
                        // always allowed
                        g_AnimationDraw.X = nNewX;
                        g_AnimationDraw.Y = nNewY;
                    }
                }else{
                    if(Fl::event_state() & FL_CTRL){
                        // bug of fltk here for windows, when some key is pressed, 
                        // event_x() and event_y() are incorrect!
                    }else{
                        // SetOffset(-(m_MouseX - mouseX), true, -(m_MouseY - mouseY), true);
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

Fl_Image *DrawArea::CreateTUC(int nIndex, bool bSelect)
{
    uint32_t nCB = (0X00000000);
    uint32_t nCF = (bSelect ? 0X800000FF : 0X8000FF00);
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

void DrawArea::DrawTUC(int nCX, int nCY, int nIndex, bool bSelect)
{
    extern EditorMap g_EditorMap;
    if(g_EditorMap.Valid() && g_EditorMap.ValidC(nCX, nCY)){
        DrawImage(m_TUC[bSelect ? 0 : 1][nIndex % 4], nCX * 48 - m_OffsetX, nCY * 32 - m_OffsetY);
    }
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

    if(g_MainWindow->SelectBySingle()){
        AddSelectBySingle();
    }

    if(g_MainWindow->SelectByRhombus()){
        AddSelectByRhombus();
    }

    if(g_MainWindow->SelectByRectangle()){
        AddSelectByRectangle();
    }

    if(g_MainWindow->SelectByAttribute()){
        AddSelectByAttribute();
    }
}

// truncate line into segment inside *DrawArea*
bool DrawArea::LocateLineSegment(int &nX1, int &nY1, int &nX2, int &nY2)
{
    // Liang-Barsky clipping algorithm.
    // https://github.com/smcameron/liang-barsky-in-c

    int nDX = nX2 - nX1;
    int nDY = nY2 - nY1;

    if(true
            && nDX == 0
            && nDY == 0
            && PointInRectangle(nX1, nY1, 0, 0, w(), h())){
        return true;
    }

    auto fnClipT = [](int nNum, int nDenom, double &ftE, double &ftL){
        if(nDenom == 0){
            return nNum < 0;
        }else{
            double fT = nNum * 1.0 / nDenom;
            if(nDenom > 0){
                if(fT > ftL){ return false; }
                if(fT > ftE){ ftE = fT    ; }
            }else{
                if(fT < ftE){ return false; }
                if(fT < ftL){ ftL = fT    ; }
            }
            return true;
        }
    };

    double ftE = 0.0;
    double ftL = 1.0;

    if(true
            && fnClipT(0 - nX1      ,  nDX, ftE, ftL)
            && fnClipT(1 + nX1 - w(), -nDX, ftE, ftL)
            && fnClipT(0 - nY1      ,  nDY, ftE, ftL)
            && fnClipT(1 + nY1 - h(), -nDY, ftE, ftL))
    {
        if(ftL < 1.0){
            nX2 = (int)std::lround(nX1 + ftL * nDX);
            nY2 = (int)std::lround(nY1 + ftL * nDY);
        }
        if(ftE > 0.0){
            nX1 += (int)std::lround(ftE * nDX);
            nY1 += (int)std::lround(ftE * nDY);
        }
        return true;
    }
    return false;
}

// coordinate (0, 0) is on most top-left of *DrawArea*
void DrawArea::DrawLine(int nAX0, int nAY0, int nAX1, int nAY1)
{
    if(LocateLineSegment(nAX0, nAY0, nAX1, nAY1)){
        fl_line(nAX0 + x(), nAY0 + y(), nAX1 + x(), nAY1 + y());
    }
}

void DrawArea::DrawRectangle(int nAX, int nAY, int nAW, int nAH)
{
    // if(nAW < 0 || nAH < 0){ return; }
    DrawLine(nAX          , nAY          , nAX + nAW - 1, nAY          );
    DrawLine(nAX          , nAY          , nAX          , nAY + nAH - 1);
    DrawLine(nAX + nAW - 1, nAY          , nAX + nAW - 1, nAY + nAH - 1);
    DrawLine(nAX          , nAY + nAH - 1, nAX + nAW - 1, nAY + nAH - 1);
}

void DrawArea::DrawLoop(int nAX1, int nAY1, int nAX2, int nAY2, int nAX3, int nAY3)
{
    DrawLine(nAX1, nAY1, nAX2, nAY2);
    DrawLine(nAX2, nAY2, nAX3, nAY3);
    DrawLine(nAX3, nAY3, nAX1, nAY1);
}

void DrawArea::AttributeCoverOperation(int nMouseXOnMap, int nMouseYOnMap, int nSize, std::function<void(int, int, int)> fnOperation)
{
    if(nSize <= 0){ return; }

    int nMX = nMouseXOnMap / 48;
    int nMY = nMouseYOnMap / 32;

    for(int nX = nMX - (nSize / 2); nX < nMX + (nSize + 1) / 2; ++nX){
        for(int nY = nMY - (nSize / 2); nY < nMY + (nSize + 1) / 2; ++nY){
            for(int nIndex = 0; nIndex < 4; ++nIndex){
                extern EditorMap g_EditorMap;
                if(g_EditorMap.ValidC(nX, nY)){
                    fnOperation(nX, nY, nIndex);
                }
            }
        }
    }
}

void DrawArea::DrawLight()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowLightLayer()){ return; }

    if(!m_LightUC){
        uint32_t pData[32][48];
        for(int nY = 0; nY < 32; ++nY){
            for(int nX = 0; nX < 48; ++nX){
                pData[nY][nX] = 0X80FF0000;
            }
        }
        m_LightUC = Fl_RGB_Image((uchar *)pData, 48, 32, 4, 0).copy(48, 32);
    }

    auto fnDrawLight = [this](int nX, int nY){
        DrawImage(m_LightUC, nX * 48 - m_OffsetX, nY * 32 - m_OffsetY);
    };

    extern EditorMap g_EditorMap;
    g_EditorMap.DrawLight(m_OffsetX / 48 - 2, m_OffsetY / 32 - 2, w() / 48 + 4, h() / 32 + 4, fnDrawLight);
}

bool DrawArea::CoverValid(int nX, int nY, int nR)
{
    extern EditorMap g_EditorMap;
    return g_EditorMap.Valid() && g_EditorMap.CoverValid(nX, nY, nR);
}

Fl_Image *DrawArea::CreateCover(int nR)
{
    uint32_t nCB = 0X00000000;
    uint32_t nCF = 0X8000090F;
    if(nR){
        int nSize = 2 * nR + 1;
        std::vector<uint32_t> bfData(nSize * nSize, nCB);
        for(int nY = 0; nY < nSize; ++nY){
            for(int nX = 0; nX < nSize; ++nX){
                if(LDistance2(nX, nY, nR, nR) <= nR * nR){
                    bfData[nY * nSize + nX] = nCF;
                }
            }
        }

        return Fl_RGB_Image((uchar *)(&bfData[0]), nSize, nSize, 4, 0).copy(nSize, nSize);
    }

    return nullptr;
}

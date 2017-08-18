/*
 * =====================================================================================
 *
 *       Filename: drawarea.cpp
 *        Created: 07/26/2015 04:27:57 AM
 *  Last Modified: 08/17/2017 18:01:57
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
    , m_RUC{nullptr, nullptr}
    , m_TextBoxBG(nullptr)
    , m_LightRUC(nullptr)
{
    m_TextBoxBG = CreateRectImage(200, 160, 0X80000000);
    m_LightRUC  = CreateRectImage(SYS_MAPGRIDXP, SYS_MAPGRIDYP, 0X80FF0000);
    m_RUC[0]    = CreateRectImage(SYS_MAPGRIDXP, SYS_MAPGRIDYP, 0X8000FF00);
    m_RUC[1]    = CreateRectImage(SYS_MAPGRIDXP, SYS_MAPGRIDYP, 0X800000FF);
}

DrawArea::~DrawArea()
{
    delete m_LightRUC;
    delete m_TextBoxBG;

    delete m_RUC[0];
    delete m_RUC[1];
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

void DrawArea::AddSelectByAttribute()
{
    auto fnSet = [](int nX, int nY)
    {
        extern EditorMap g_EditorMap;
        if(g_EditorMap.ValidC(nX, nY)){
            extern AttributeSelectWindow *g_AttributeSelectWindow;
            if(g_AttributeSelectWindow->TestLand(g_EditorMap.Cell(nX, nY).MakeLandU8())){
                extern MainWindow *g_MainWindow;
                g_EditorMap.Cell(nX, nY).SelectGround = g_MainWindow->Deselect() ? false : true;
            }
        }
    };

    int nMX = m_MouseX + m_OffsetX - x();
    int nMY = m_MouseY + m_OffsetY - y();

    extern SelectSettingWindow *g_SelectSettingWindow;
    AttributeCoverOperation(nMX, nMY, g_SelectSettingWindow->AttributeSize(), fnSet);
}

void DrawArea::DrawSelectBySingle()
{
    int nX = (m_MouseX - x() + m_OffsetX) / SYS_MAPGRIDXP;
    int nY = (m_MouseY - y() + m_OffsetY) / SYS_MAPGRIDYP;

    extern MainWindow *g_MainWindow;
    DrawRUC(nX, nY, !g_MainWindow->Deselect());
}

void DrawArea::AddSelectBySingle()
{
    int nX = (m_MouseX - x() + m_OffsetX) / SYS_MAPGRIDXP;
    int nY = (m_MouseY - y() + m_OffsetY) / SYS_MAPGRIDYP;

    extern EditorMap g_EditorMap;
    if(g_EditorMap.ValidC(nX, nY)){
        extern MainWindow *g_MainWindow;
        g_EditorMap.Cell(nX, nY).SelectGround = (g_MainWindow->Deselect() ? false : true);
    }
}

void DrawArea::RhombusCoverOperation(int nMX, int nMY, int nSize, std::function<void(int, int)> fnOperation)
{
    if(nSize <= 0){ return; }

    // don't check boundary condition
    // since even center point is out of map
    // we can still select grids over map

    int nCX = nMX / SYS_MAPGRIDXP;
    int nCY = nMY / SYS_MAPGRIDYP - nSize / 2;

    // mode 0: 0, 2
    // mode 1: 1, 3

    int nStartMode = (2 * (nMX % SYS_MAPGRIDXP) < 3 * (nMY % SYS_MAPGRIDYP) ) ? 0 : 1;
    int nMode = nStartMode;
    int nLine = 1;

    auto fnLineCoverInfo = [nCX, nCY, nStartMode](int &nStartX, int &nStartY, int &nCnt, int nLine)
    {
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
                fnOperation(nStartX + nIndex, nStartY    );
                fnOperation(nStartX + nIndex, nStartY + 1);
            }else{
                fnOperation(nStartX + nIndex    , nStartY);
                fnOperation(nStartX + nIndex + 1, nStartY);
            }
        }
        nMode = 1 - nMode;
        nLine++;
    }
}

void DrawArea::DrawSelectByRhombus()
{
    auto fnDraw = [this](int nX,  int nY)
    {
        extern MainWindow *g_MainWindow;
        DrawRUC(nX, nY, !g_MainWindow->Deselect());
    };

    int nMX = m_MouseX + m_OffsetX - x();
    int nMY = m_MouseY + m_OffsetY - y();

    extern SelectSettingWindow *g_SelectSettingWindow;
    RhombusCoverOperation(nMX, nMY, g_SelectSettingWindow->RhombusSize(), fnDraw);
}

void DrawArea::AddSelectByRhombus()
{
    auto fnSet = [](int nX, int nY)
    {
        extern EditorMap g_EditorMap;
        if(g_EditorMap.ValidC(nX, nY)){
            extern MainWindow *g_MainWindow;
            g_EditorMap.Cell(nX, nY).SelectGround = g_MainWindow->Deselect() ? false : true;
        }
    };

    int nMX = m_MouseX + m_OffsetX - x();
    int nMY = m_MouseY + m_OffsetY - y();

    extern SelectSettingWindow *g_SelectSettingWindow;
    RhombusCoverOperation(nMX, nMY, g_SelectSettingWindow->RhombusSize(), fnSet);
}

void DrawArea::RectangleCoverOperation(int nMouseXOnMap, int nMouseYOnMap, int nSize, std::function<void(int, int)> fnOperation)
{
    if(nSize > 0){
        int nMX = nMouseXOnMap / SYS_MAPGRIDXP - nSize / 2;
        int nMY = nMouseYOnMap / SYS_MAPGRIDYP - nSize / 2;

        for(int nX = 0; nX < nSize; ++nX){
            for(int nY = 0; nY < nSize; ++nY){
                extern EditorMap g_EditorMap;
                if(g_EditorMap.ValidC(nX + nMX, nY + nMY)){
                    fnOperation(nX + nMX, nY + nMY);
                }
            }
        }
    }
}

void DrawArea::DrawSelectByRectangle()
{
    auto fnDraw = [this](int nX,  int nY)
    {
        extern MainWindow *g_MainWindow;
        DrawRUC(nX, nY, !g_MainWindow->Deselect());
    };

    int nMX = m_MouseX + m_OffsetX - x();
    int nMY = m_MouseY + m_OffsetY - y();

    extern SelectSettingWindow *g_SelectSettingWindow;
    RectangleCoverOperation(nMX, nMY, g_SelectSettingWindow->RectangleSize(), fnDraw);
}

void DrawArea::AddSelectByRectangle()
{
    auto fnSet = [](int nX,  int nY)
    {
        extern EditorMap g_EditorMap;
        extern MainWindow *g_MainWindow;
        g_EditorMap.Cell(nX, nY).SelectGround = g_MainWindow->Deselect() ? false : true;
    };

    int nMX = m_MouseX + m_OffsetX - x();
    int nMY = m_MouseY + m_OffsetY - y();

    extern SelectSettingWindow *g_SelectSettingWindow;
    RectangleCoverOperation(nMX, nMY, g_SelectSettingWindow->RectangleSize(), fnSet);
}

void DrawArea::DrawSelectByAttribute()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->AttributeSize();

    if(nSize <= 0){ return; }

    int nMX = m_MouseX + m_OffsetX - x();
    int nMY = m_MouseY + m_OffsetY - y();

    auto fnDraw = [this](int nX, int nY) -> void
    {
        extern EditorMap g_EditorMap;
        extern MainWindow *g_MainWindow;
        extern AttributeSelectWindow *g_AttributeSelectWindow;

        if(g_AttributeSelectWindow->TestLand(g_EditorMap.Cell(nX, nY).MakeLandU8())){
            DrawRUC(nX, nY, !g_MainWindow->Deselect());
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

    auto nColor = fl_color();
    fl_color(FL_YELLOW);
    DrawRectangle(nAX, nAY, nAW, nAH);
    fl_color(nColor);
}

void DrawArea::DrawSelect()
{
    int nX = m_OffsetX / SYS_MAPGRIDXP - 1;
    int nY = m_OffsetY / SYS_MAPGRIDYP - 1;

    int nXSize = w() / SYS_MAPGRIDXP + 3;
    int nYSize = h() / SYS_MAPGRIDYP + 3;

    for(int nTX = nX; nTX < nXSize + nX; ++nTX){
        for(int nTY = nY; nTY < nYSize + nY; ++nTY){
            extern EditorMap g_EditorMap;
            if(g_EditorMap.ValidC(nTX, nTY)){
                extern MainWindow *g_MainWindow;
                if(g_EditorMap.Cell(nTX, nTY).SelectGround == !g_MainWindow->Reversed()){
                    DrawRUC(nTX, nTY, !g_MainWindow->Reversed());
                }
            }
        }
    }
}

void DrawArea::DrawTrySelect()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->EnableSelect()){
        auto nColor = fl_color();
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

        fl_color(nColor);
    }
}

void DrawArea::DrawTextBox()
{
    DrawImage(m_TextBoxBG, 0, 0);

    auto nColor = fl_color();
    fl_color(FL_RED);

    char szInfo[128];
    std::sprintf(szInfo, "OffsetX: %d %d", m_OffsetX / SYS_MAPGRIDXP, m_OffsetX);
    fl_draw(szInfo, 10 + x(), 20 + y());

    std::sprintf(szInfo, "OffsetY: %d %d", m_OffsetY / SYS_MAPGRIDYP, m_OffsetY);
    fl_draw(szInfo, 10 + x(), 40 + y());

    int nMX = std::max(0, m_MouseX + m_OffsetX - x());
    int nMY = std::max(0, m_MouseY + m_OffsetY - y());

    std::sprintf(szInfo, "MouseMX: %d %d", nMX / SYS_MAPGRIDXP, nMX);
    fl_draw(szInfo, 10 + x(), 60 + y());

    std::sprintf(szInfo, "MouseMY: %d %d", nMY / SYS_MAPGRIDYP, nMY);
    fl_draw(szInfo, 10 + x(), 80 + y());

    fl_color(nColor);
}

void DrawArea::DrawObject(bool bGround)
{
    extern MainWindow *g_MainWindow;
    auto nColor = fl_color();
    if(false
            || ( bGround && g_MainWindow->ShowGroundObjectLayer())
            || (!bGround && g_MainWindow->ShowOverGroundObjectLayer())){

        fl_color(bGround ? FL_BLUE : FL_GREEN);
        auto fnDrawExt = [this, bGround](int /* nXCnt */, int /* nYCnt */) -> void
        {
            // if(!bGround){
            //     extern MainWindow *g_MainWindow;
            //     if(g_MainWindow->EnableTest()){
            //         extern AnimationDB g_AnimationDB;
            //         extern AnimationDraw g_AnimationDraw;
            //
            //         if(g_AnimationDraw.MonsterID){
            //             auto &rstAnimation = g_AnimationDB.RetrieveAnimation(g_AnimationDraw.MonsterID);
            //             if(true
            //                     && g_AnimationDraw.X / SYS_MAPGRIDXP == nXCnt
            //                     && g_AnimationDraw.Y / SYS_MAPGRIDYP == nYCnt){
            //                 if(rstAnimation.ResetFrame(g_AnimationDraw.Action, g_AnimationDraw.Direction, g_AnimationDraw.Frame)){
            //                     auto fnDraw = [this](Fl_Shared_Image *pPNG, int nMapX, int nMapY) -> void
            //                     {
            //                         DrawImage(pPNG, nMapX - m_OffsetX, nMapY - m_OffsetY);
            //                     };
            //
            //                     // use R from the AnimationSelectWindow, rather than the copy in AnimationDraw
            //                     // this enables me to adjust R without reset the animation MonsterID
            //                     extern AnimationSelectWindow *g_AnimationSelectWindow;
            //                     int nAnimationX = g_AnimationDraw.X - m_OffsetX;
            //                     int nAnimationY = g_AnimationDraw.Y - m_OffsetY;
            //                     int nAnimationR = g_AnimationSelectWindow->R();
            //
            //                     // draw funcitons take coordinates w.r.t the window rather than the widget
            //                     // fl_circle(x() + nAnimationX * 1.0, y() + nAnimationY * 1.0, nAnimationR * 1.0);
            //                     DrawImage(m_CoverV[nAnimationR], nAnimationX - nAnimationR, nAnimationY - nAnimationR);
            //                     fl_circle(x() + nAnimationX * 1.0, y() + nAnimationY * 1.0, nAnimationR * 1.0);
            //                     rstAnimation.Draw(g_AnimationDraw.X, g_AnimationDraw.Y, fnDraw);
            //                 }
            //             }
            //         }
            //     }
            // }
        };

        auto fnDrawObj = [this, bGround](uint8_t nFileIndex, uint16_t nImageIndex, int nXCnt, int nYCnt) -> void
        {
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
        g_EditorMap.DrawObject(m_OffsetX / SYS_MAPGRIDXP - 10, m_OffsetY / SYS_MAPGRIDYP - 20, w() / SYS_MAPGRIDXP + 20, h() / SYS_MAPGRIDYP + 40, bGround, fnDrawObj, fnDrawExt);

        fl_color(nColor);
    }
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
    if(g_MainWindow->ShowAttributeGridLine()){
        auto nColor = fl_color();
        fl_color(FL_MAGENTA);
        for(int nCX = m_OffsetX / SYS_MAPGRIDXP - 1; nCX < (m_OffsetX + w()) / SYS_MAPGRIDXP + 1; ++nCX){
            for(int nCY = m_OffsetY / SYS_MAPGRIDYP - 1; nCY < (m_OffsetY + h()) / SYS_MAPGRIDYP + 1; ++nCY){
                extern EditorMap g_EditorMap;
                if(g_EditorMap.ValidC(nCX, nCY)){
                    extern AttributeSelectWindow *g_AttributeSelectWindow;
                    if(g_AttributeSelectWindow->TestLand(g_EditorMap.Cell(nCX, nCY).MakeLandU8())){
                        int nPX = nCX * SYS_MAPGRIDXP - m_OffsetX;
                        int nPY = nCY * SYS_MAPGRIDYP - m_OffsetY;
                        DrawRectangle(nPX, nPY, SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                        DrawLine(nPX, nPY, nPX + SYS_MAPGRIDXP, nPY + SYS_MAPGRIDYP);
                        DrawLine(nPX + SYS_MAPGRIDXP, nPY, nPX, nPY + SYS_MAPGRIDYP);
                    }
                }
            }
        }
        fl_color(nColor);
    }
}

void DrawArea::DrawGrid()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ShowGridLine()){
        auto nColor = fl_color();
        fl_color(FL_MAGENTA);

        for(int nCX = m_OffsetX / SYS_MAPGRIDXP - 1; nCX < (m_OffsetX + w()) / SYS_MAPGRIDXP + 1; ++nCX){
            DrawLine(nCX * SYS_MAPGRIDXP - m_OffsetX, 0, nCX * SYS_MAPGRIDXP - m_OffsetX, h());
        }

        for(int nCY = m_OffsetY / SYS_MAPGRIDYP - 1; nCY < (m_OffsetY + h()) / SYS_MAPGRIDYP + 1; ++nCY){
            DrawLine(0, nCY * SYS_MAPGRIDYP - m_OffsetY, w(), nCY * SYS_MAPGRIDYP - m_OffsetY);
        }
        fl_color(nColor);
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
            g_ImageCache.Register(nFileIndex, nImageIndex, g_ImageDB.FastDecode(nFileIndex, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF), nW, nH);
            p = g_ImageCache.Retrieve(nFileIndex, nImageIndex);
        }
    }
    return p;
}

void DrawArea::DrawTile()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ShowBaseTileLayer()){
        auto nColor = fl_color();
        fl_color(FL_RED);

        auto fnDraw = [this](uint8_t nFileIndex, uint16_t nImageIndex, int nX, int nY) -> void
        {
            int nStartX = nX * SYS_MAPGRIDXP - m_OffsetX;
            int nStartY = nY * SYS_MAPGRIDYP - m_OffsetY;
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
        g_EditorMap.DrawTile(m_OffsetX / SYS_MAPGRIDXP - 5, m_OffsetY / SYS_MAPGRIDYP - 5, w() / SYS_MAPGRIDXP + 10, h() / SYS_MAPGRIDYP + 10, fnDraw);
        fl_color(nColor);
    }
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
    m_OffsetY = (std::max)(m_OffsetY, 0);
    m_OffsetY = (std::min)(m_OffsetY, (std::max)(0, 32 * g_EditorMap.H() - h()));
}

int DrawArea::handle(int nEvent)
{
    extern EditorMap g_EditorMap;
    auto nRet = Fl_Box::handle(nEvent);

    if(g_EditorMap.Valid()){

        int nMouseX = m_MouseX;
        int nMouseY = m_MouseY;
        m_MouseX    = Fl::event_x();
        m_MouseY    = Fl::event_y();

        switch(nEvent){
            case FL_RELEASE:
                {
                    fl_cursor(FL_CURSOR_DEFAULT);
                    break;
                }

            case FL_MOVE:
                {
                    break;
                }

            case FL_DRAG:
                {
                    extern MainWindow *g_MainWindow;
                    if(g_MainWindow->EnableSelect()){
                        AddSelect();
                    }else if(g_MainWindow->EnableEdit()){
                        // TODO
                        //
                    }else if(g_MainWindow->EnableTest()){
                        // // we are moving the animation to a proper place
                        // // if current position is invalid, then we permit any moving to get a valid
                        // // position, but if current position is valid, we reject any move request
                        // // which make the position invlaid again
                        //
                        // extern AnimationDraw g_AnimationDraw;
                        // extern AnimationSelectWindow *g_AnimationSelectWindow;
                        // int nNewX = g_AnimationDraw.X + (m_MouseX - nMouseX);
                        // int nNewY = g_AnimationDraw.Y + (m_MouseY - nMouseY);
                        //
                        // if(CoverValid(g_AnimationDraw.X, g_AnimationDraw.Y, g_AnimationSelectWindow->R())){
                        //     if(CoverValid(nNewX, nNewY, g_AnimationSelectWindow->R())){
                        //         g_AnimationDraw.X = nNewX;
                        //         g_AnimationDraw.Y = nNewY;
                        //     }else{
                        //         // try to find a feasible internal point by binary search
                        //         int nX0 = g_AnimationDraw.X;
                        //         int nY0 = g_AnimationDraw.Y;
                        //         int nX1 = nNewX;
                        //         int nY1 = nNewY;
                        //         while((std::abs(nX1 - nX0) >= 2) || (std::abs(nY1 - nY0) >= 2)){
                        //             int nMidX = (nX0 + nX1) / 2;
                        //             int nMidY = (nY0 + nY1) / 2;
                        //
                        //             if(CoverValid(nMidX, nMidY, g_AnimationSelectWindow->R())){
                        //                 nX0 = nMidX;
                        //                 nY0 = nMidY;
                        //             }else{
                        //                 nX1 = nMidX;
                        //                 nY1 = nMidY;
                        //             }
                        //         }
                        //         g_AnimationDraw.X = nX0;
                        //         g_AnimationDraw.Y = nY0;
                        //     }
                        // }else{
                        //     // always allowed
                        //     g_AnimationDraw.X = nNewX;
                        //     g_AnimationDraw.Y = nNewY;
                        // }
                    }else{
                        if(Fl::event_state() & FL_CTRL){
                            // bug of fltk here for windows, when some key is pressed, 
                            // event_x() and event_y() are incorrect!
                        }else{
                            // SetOffset(-(m_MouseX - nMouseX), true, -(m_MouseY - nMouseY), true);
                            SetOffset(-(m_MouseX - nMouseX), true, -(m_MouseY - nMouseY), true);
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
                nRet = 1;
                break;
            default:
                break;
        }

    }

    extern MainWindow *g_MainWindow;
    g_MainWindow->RedrawAll();

    return nRet;
}

Fl_Image *DrawArea::CreateRectImage(int nW, int nH, uint32_t nColor)
{
    if(nW > 0 && nH > 0){
        std::vector<uint32_t> stvBuf(nW * nH, nColor);
        return Fl_RGB_Image((uchar *)(&(stvBuf[0])), nW, nH, 4, 0).copy(nW, nH);
    }else{
        fl_alert("Invalid size for CreateRectImage(%d, %d, 0X%08X)", nW, nH, nColor);
        return nullptr;
    }
}

void DrawArea::DrawRUC(int nCX, int nCY, bool bSelect)
{
    extern EditorMap g_EditorMap;
    if(g_EditorMap.Valid() && g_EditorMap.ValidC(nCX, nCY)){
        DrawImage(m_RUC[bSelect ? 0 : 1], nCX * SYS_MAPGRIDXP - m_OffsetX, nCY * SYS_MAPGRIDYP - m_OffsetY);
    }
}

void DrawArea::ClearGroundSelect()
{
    extern EditorMap g_EditorMap;
    if(g_EditorMap.Valid()){
        g_EditorMap.ClearGroundSelect();
    }
}

void DrawArea::SetScrollBar()
{
    double fXP = -1.0;
    double fYP = -1.0;

    extern EditorMap g_EditorMap;
    if(SYS_MAPGRIDXP * g_EditorMap.W() - w() > 0){
        fXP = m_OffsetX * 1.0 / (SYS_MAPGRIDXP * g_EditorMap.W()  - w());
    }
    if(SYS_MAPGRIDYP * g_EditorMap.H() - h() > 0){
        fYP = m_OffsetY * 1.0 / (SYS_MAPGRIDYP * g_EditorMap.H() - h());
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
// very useful and should move this function to mathfunc.hpp
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

    auto fnClipT = [](int nNum, int nDenom, double &ftE, double &ftL) -> bool
    {
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

void DrawArea::AttributeCoverOperation(int nMouseXOnMap, int nMouseYOnMap, int nSize, std::function<void(int, int)> fnOperation)
{
    if(nSize> 0){
        int nMX = nMouseXOnMap / SYS_MAPGRIDXP;
        int nMY = nMouseYOnMap / SYS_MAPGRIDYP;

        for(int nX = nMX - (nSize / 2); nX < nMX + (nSize + 1) / 2; ++nX){
            for(int nY = nMY - (nSize / 2); nY < nMY + (nSize + 1) / 2; ++nY){
                extern EditorMap g_EditorMap;
                if(g_EditorMap.ValidC(nX, nY)){
                    fnOperation(nX, nY);
                }
            }
        }
    }
}

void DrawArea::DrawLight()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ShowLightLayer()){
        auto fnDrawLight = [this](int nX, int nY) -> void
        {
            DrawImage(m_LightRUC, nX * SYS_MAPGRIDXP - m_OffsetX, nY * SYS_MAPGRIDYP - m_OffsetY);
        };

        extern EditorMap g_EditorMap;
        g_EditorMap.DrawLight(m_OffsetX / SYS_MAPGRIDXP - 2, m_OffsetY / SYS_MAPGRIDYP - 2, w() / SYS_MAPGRIDXP + 4, h() / SYS_MAPGRIDYP + 4, fnDrawLight);
    }
}

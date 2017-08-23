/*
 * =====================================================================================
 *
 *       Filename: drawarea.cpp
 *        Created: 07/26/2015 04:27:57 AM
 *  Last Modified: 08/22/2017 17:25:13
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

#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <functional>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Shared_Image.H>

#include "mir2map.hpp"
#include "imagedb.hpp"
#include "drawarea.hpp"
#include "sysconst.hpp"
#include "mathfunc.hpp"
#include "editormap.hpp"
#include "colorfunc.hpp"
#include "animation.hpp"
#include "mainwindow.hpp"
#include "imagecache.hpp"
#include "animationdb.hpp"
#include "animationdraw.hpp"
#include "animationselectwindow.hpp"
#include "attributeselectwindow.hpp"

DrawArea::DrawArea(int x, int y, int w, int h)
    : Fl_Box(x, y, w, h)
    , m_MouseX(0)
    , m_MouseY(0)
    , m_OffsetX(0)
    , m_OffsetY(0)
    , m_RUC{nullptr, nullptr}
    , m_LightRUC(nullptr)
    , m_LightImge(nullptr)
    , m_TextBoxBG(nullptr)
    , m_FloatObjectBG(nullptr)
{
    m_RUC[0]    = CreateRectImage(SYS_MAPGRIDXP, SYS_MAPGRIDYP, 0X8000FF00);
    m_RUC[1]    = CreateRectImage(SYS_MAPGRIDXP, SYS_MAPGRIDYP, 0X800000FF);
    m_LightRUC  = CreateRectImage(SYS_MAPGRIDXP, SYS_MAPGRIDYP, 0X80FF0000);
    m_LightImge = CreateRoundImage(200, 0X001286FF);
    m_TextBoxBG = CreateRectImage(200, 160, 0X80000000);
    m_FloatObjectBG = CreateRectImage(SYS_MAPGRIDXP, SYS_MAPGRIDYP, 0XC0000000);
}

DrawArea::~DrawArea()
{
    delete m_LightRUC;
    delete m_TextBoxBG;
    delete m_LightImge;
    delete m_FloatObjectBG;

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

void DrawArea::DrawSelectByTile()
{
    int nX = ((m_MouseX - x() + m_OffsetX) / (2 * SYS_MAPGRIDXP)) * 2;
    int nY = ((m_MouseY - y() + m_OffsetY) / (2 * SYS_MAPGRIDYP)) * 2;

    extern MainWindow *g_MainWindow;
    DrawRUC(nX,     nY,     !g_MainWindow->Deselect());
    DrawRUC(nX + 1, nY,     !g_MainWindow->Deselect());
    DrawRUC(nX,     nY + 1, !g_MainWindow->Deselect());
    DrawRUC(nX + 1, nY + 1, !g_MainWindow->Deselect());
}

void DrawArea::DrawSelectByObjectGround(bool bGround)
{
    int nMouseXOnMap = m_MouseX - x() + m_OffsetX;
    int nMouseYOnMap = m_MouseY - y() + m_OffsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    extern EditorMap g_EditorMap;
    for(int nCurrY = nY - 1; nCurrY < g_EditorMap.H(); ++nCurrY){
        if(g_EditorMap.ValidC(nX, nCurrY)){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                auto &rstObject = g_EditorMap.Object(nX, nCurrY, nIndex);
                if(true
                        && rstObject.Valid
                        && rstObject.Ground == bGround){

                    auto nFileIndex  = (uint8_t )((rstObject.Image & 0X00FF0000) >> 16);
                    auto nImageIndex = (uint16_t)((rstObject.Image & 0X0000FFFF) >>  0);

                    if(auto pImage = RetrievePNG(nFileIndex, nImageIndex)){
                        int nW = pImage->w();
                        int nH = pImage->h();

                        int nStartX = nX     * SYS_MAPGRIDXP - m_OffsetX;
                        int nStartY = nCurrY * SYS_MAPGRIDYP - m_OffsetY + SYS_MAPGRIDYP - nH;

                        if(PointInRectangle(nMouseXOnMap - m_OffsetX, nMouseYOnMap - m_OffsetY, nStartX, nStartY, nW, nH)){
                            extern MainWindow *g_MainWindow;
                            DrawImageCover(m_RUC[g_MainWindow->Deselect() ? 1 : 0], nStartX, nStartY, nW, nH);
                            DrawFloatObject(nX, nCurrY, (nIndex == 0) ? FOTYPE_OBJ0 : FOTYPE_OBJ1, m_MouseX - x(), m_MouseY - y());
                            return;
                        }
                    }
                }
            }
        }
    }
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

        if(g_MainWindow->SelectByTile()){
            DrawSelectByTile();
        }

        if(g_MainWindow->SelectByObjectGround()){
            DrawSelectByObjectGround(true);
        }

        if(g_MainWindow->SelectByObjectOverGround()){
            DrawSelectByObjectGround(false);
        }

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

    int nY = y() + 20;

    // offset (x, y)
    {
        char szInfo[128];

        std::sprintf(szInfo, "OffsetX: %d %d", m_OffsetX / SYS_MAPGRIDXP, m_OffsetX);
        fl_draw(szInfo, 10 + x(), nY); nY += 20;

        std::sprintf(szInfo, "OffsetY: %d %d", m_OffsetY / SYS_MAPGRIDYP, m_OffsetY);
        fl_draw(szInfo, 10 + x(), nY); nY += 20;
    }

    // mouse (x, y)
    {
        char szInfo[128];
        int nMX = std::max(0, m_MouseX + m_OffsetX - x());
        int nMY = std::max(0, m_MouseY + m_OffsetY - y());

        std::sprintf(szInfo, "MouseMX: %d %d", nMX / SYS_MAPGRIDXP, nMX);
        fl_draw(szInfo, 10 + x(), nY); nY += 20;

        std::sprintf(szInfo, "MouseMY: %d %d", nMY / SYS_MAPGRIDYP, nMY);
        fl_draw(szInfo, 10 + x(), nY); nY += 20;
    }

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

void DrawArea::DrawImage(Fl_Image *pImage, int nAX, int nAY, int nImageX, int nImageY, int nImageW, int nImageH)
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
                && RectangleOverlapRegion<int>(0, 0, pImage->w(), pImage->h(), &nImageX, &nImageY, &nImageW, &nImageH)){

            nAX += (nImageX - nOldImageX);
            nAY += (nImageY - nOldImageY);

            auto nOldAX = nAX;
            auto nOldAY = nAY;

            if(true
                    && nImageW > 0
                    && nImageH > 0
                    && RectangleOverlapRegion<int>(0, 0, w(), h(), &nAX, &nAY, &nImageW, &nImageH)){

                nImageX += (nAX - nOldAX);
                nImageY += (nAY - nOldAY);

                if(true
                        && nImageW > 0
                        && nImageH > 0){

                    pImage->draw(nAX + x(), nAY + y(), nImageW, nImageH, nImageX, nImageY);
                }
            }
        }
    }
}

void DrawArea::DrawImage(Fl_Image *pImage, int nAX, int nAY)
{
    if(pImage){
        DrawImage(pImage, nAX, nAY, 0, 0, pImage->w(), pImage->h());
    }
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
            case FL_MOUSEWHEEL:
                {
                    int nDX = Fl::event_dx() * SYS_MAPGRIDXP;
                    int nDY = Fl::event_dy() * SYS_MAPGRIDYP;

                    SetOffset(nDX, true, nDY, true);
                    nRet = 1;
                    break;
                }
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

                    break;
                }
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

                    // for drag nEvent
                    nRet = 1;
                    break;
                }
            default:
                {
                    break;
                }
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

Fl_Image *DrawArea::CreateRoundImage(int nRadius, uint32_t nColor)
{
    if(nRadius > 0){
        int nSize = 1 + (nRadius - 1) * 2;
        std::vector<uint32_t> stvBuf(nSize * nSize, 0X00000000);
        for(int nX = 0; nX < nSize; ++nX){
            for(int nY = 0; nY < nSize; ++nY){
                auto nRX = nX - nRadius + 1;
                auto nRY = nY - nRadius + 1;
                auto nR2 = LDistance2<int>(nRX, nRY, 0, 0);
                if(nR2 <= nRadius * nRadius){
                    auto nA = (uint8_t)(0X02 + std::lround(0X2F * (1.0 - 1.0 * nR2 / (nRadius * nRadius))));
                    stvBuf[nX + nY * nSize] = (((uint32_t)(nA)) << 24) | (nColor & 0X00FFFFFF);
                }
            }
        }
        return Fl_RGB_Image((uchar *)(&(stvBuf[0])), nSize, nSize, 4, 0).copy(nSize, nSize);
    }else{
        fl_alert("Invalid radius for CreateRoundImage(%d, 0X%08X)", nRadius, nColor);
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
            // DrawImage(m_LightRUC, nX * SYS_MAPGRIDXP - m_OffsetX, nY * SYS_MAPGRIDYP - m_OffsetY);
            DrawImage(m_LightImge,
                    nX * SYS_MAPGRIDXP - m_OffsetX + SYS_MAPGRIDXP / 2 - (m_LightImge->w() - 1) / 2,
                    nY * SYS_MAPGRIDYP - m_OffsetY + SYS_MAPGRIDYP / 2 - (m_LightImge->h() - 1) / 2);
        };

        extern EditorMap g_EditorMap;
        g_EditorMap.DrawLight(m_OffsetX / SYS_MAPGRIDXP - 2, m_OffsetY / SYS_MAPGRIDYP - 2, w() / SYS_MAPGRIDXP + 4, h() / SYS_MAPGRIDYP + 4, fnDrawLight);
    }
}

void DrawArea::DrawImageCover(Fl_Image *pImage, int nX, int nY, int nW, int nH)
{
    if(true
            && pImage
            && pImage->w() > 0
            && pImage->h() > 0

            && nW > 0
            && nH > 0
            && RectangleOverlapRegion(0, 0, w(), h(), &nX, &nY, &nW, &nH)){

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

void DrawArea::DrawFloatObject(int nX, int nY, int nFOType, int nWinX, int nWinY)
{
    // draw a window with detailed information
    // +----------------------------------+
    // |     0      +------------------+  |
    // |  +-----+   |                  |  |
    // |  |     |   |                  |  |
    // |  |  1  |   |         2        |  |
    // |  |     |   |                  |  |
    // |  |     |   |                  |  |
    // |  +-----+   +------------------+  |
    // |                                  |
    // +----------------------------------+

    // 0: container
    // 1: image
    // 2: text

    // 0: (0 -> WinX, 0 -> WinY, WinW, WinH)
    // 1: (ImageX, ImageY, image::w(), image::h())
    // 2: (TextBoxX, TextBoxY, TextBoxW, TextBoxH)

    extern EditorMap g_EditorMap;
    if(true
            && nFOType > FOTYPE_NONE
            && nFOType < FOTYPE_MAX
            && g_EditorMap.ValidC(nX, nY)){

        // for different FOType
        // we have different text box size

        int nTextBoxW = -1;
        int nTextBoxH = -1;

        uint8_t  nFileIndex  = 0;
        uint16_t nImageIndex = 0;

        Fl_Image *pImage = nullptr;
        switch(nFOType){
            case FOTYPE_TILE:
                {
                    nTextBoxW = 200;
                    nTextBoxH =  80;

                    if(true
                            && !(nX % 2)
                            && !(nY % 2)){

                        auto &rstTile = g_EditorMap.Tile(nX, nY);
                        if(rstTile.Valid){
                            nFileIndex  = (uint8_t )((rstTile.Image & 0X00FF0000) >> 16);
                            nImageIndex = (uint16_t)((rstTile.Image & 0X0000FFFF) >>  0);
                            pImage = RetrievePNG(nFileIndex, nImageIndex);
                        }
                    }
                    break;
                }
            case FOTYPE_OBJ0:
            case FOTYPE_OBJ1:
                {
                    nTextBoxW = 200;
                    nTextBoxH =  80;

                    auto &rstObject = g_EditorMap.Object(nX, nY, (nFOType == FOTYPE_OBJ0) ? 0 : 1);
                    if(rstObject.Valid){
                        nFileIndex  = (uint8_t )((rstObject.Image & 0X00FF0000) >> 16);
                        nImageIndex = (uint16_t)((rstObject.Image & 0X0000FFFF) >>  0);
                        pImage = RetrievePNG(nFileIndex, nImageIndex);
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }

        if(true
                && pImage
                && pImage->w() > 0
                && pImage->h() > 0){

            int nMarginTop    = 30;
            int nMarginBottom = 30;

            int nMarginLeft   = 30;
            int nMarginMiddle = 30;
            int nMarginRight  = 30;

            int nWinW = nMarginLeft + nMarginRight  + (pImage->w() + nTextBoxW) + nMarginMiddle;
            int nWinH = nMarginTop  + nMarginBottom + (std::max<int>)(pImage->h(), nTextBoxH);

            int nImageX = nWinX + nMarginLeft;
            int nImageY = nWinY + (nWinH - pImage->h()) / 2;

            int nTextBoxX = nWinX + nMarginLeft + pImage->w() + nMarginMiddle;
            int nTextBoxY = nWinY + (nWinH - nTextBoxH) / 2;

            DrawImageCover(m_FloatObjectBG, nWinX, nWinY, nWinW, nWinH);
            DrawImage(pImage, nImageX, nImageY);

            // draw boundary for window
            {
                auto nColor = fl_color();
                fl_color(FL_YELLOW);
                DrawRectangle(nWinX, nWinY, nWinW, nWinH);
                fl_color(nColor);
            }

            // draw boundary for image
            {
                auto nColor = fl_color();
                fl_color(FL_MAGENTA);
                DrawRectangle(nImageX, nImageY, pImage->w(), pImage->h());
                fl_color(nColor);
            }

            // draw textbox
            // after we allocated textbox
            // we have small offset to start inside it

            int nTextOffX = 0;
            int nTextOffY = 20;

            switch(nFOType){
                case FOTYPE_TILE:
                    {
                        // draw type
                        {
                            char szInfo[128];
                            std::sprintf(szInfo, "     Tile");
                            fl_draw(szInfo, x() + nTextBoxX + nTextOffX, y() + nTextBoxY + nTextOffY);
                            nTextOffY += 20;
                        }

                        // draw fileindex
                        {
                            char szInfo[128];
                            std::sprintf(szInfo, "Index0 : %d", (int)(nFileIndex));
                            fl_draw(szInfo, x() + nTextBoxX + nTextOffX, y() + nTextBoxY + nTextOffY);
                            nTextOffY += 20;
                        }

                        // draw imageindex
                        {
                            char szInfo[128];
                            std::sprintf(szInfo, "Index1 : %d", (int)(nImageIndex));
                            fl_draw(szInfo, x() + nTextBoxX + nTextOffX, y() + nTextBoxY + nTextOffY);
                            nTextOffY += 20;
                        }

                        // draw filename
                        {
                            char szInfo[128];
                            extern ImageDB g_ImageDB;
                            std::sprintf(szInfo, "DBName : %s", g_ImageDB.DBName(nFileIndex));
                            fl_draw(szInfo, x() + nTextBoxX + nTextOffX, y() + nTextBoxY + nTextOffY);
                            nTextOffY += 20;
                        }


                        // draw animation info
                        {
                        }

                        // dra alpha info
                        {
                        }
                        break;
                    }
                case FOTYPE_OBJ0:
                case FOTYPE_OBJ1:
                    {
                        // draw type
                        {
                            char szInfo[128];
                            std::sprintf(szInfo, "   Object[%d]", nFOType == (FOTYPE_OBJ0) ? 0 : 1);
                            fl_draw(szInfo, x() + nTextBoxX + nTextOffX, y() + nTextBoxY + nTextOffY);
                            nTextOffY += 20;
                        }

                        // draw fileindex
                        {
                            char szInfo[128];
                            std::sprintf(szInfo, "Index0 : %d", (int)(nFileIndex));
                            fl_draw(szInfo, x() + nTextBoxX + nTextOffX, y() + nTextBoxY + nTextOffY);
                            nTextOffY += 20;
                        }

                        // draw imageindex
                        {
                            char szInfo[128];
                            std::sprintf(szInfo, "Index1 : %d", (int)(nImageIndex));
                            fl_draw(szInfo, x() + nTextBoxX + nTextOffX, y() + nTextBoxY + nTextOffY);
                            nTextOffY += 20;
                        }

                        // draw filename
                        {
                            char szInfo[128];
                            extern ImageDB g_ImageDB;
                            std::sprintf(szInfo, "DBName : %s", g_ImageDB.DBName(nFileIndex));
                            fl_draw(szInfo, x() + nTextBoxX + nTextOffX, y() + nTextBoxY + nTextOffY);
                            nTextOffY += 20;
                        }


                        // draw animation info
                        {
                        }

                        // dra alpha info
                        {
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }
}

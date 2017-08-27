/*
 * =====================================================================================
 *
 *       Filename: drawarea.cpp
 *        Created: 07/26/2017 04:27:57
 *  Last Modified: 08/26/2017 23:07:02
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

DrawArea::DrawArea(int nX, int nY, int nW, int nH)
    : BaseArea(nX, nY, nW, nH)
    , m_MouseX(0)
    , m_MouseY(0)
    , m_OffsetX(0)
    , m_OffsetY(0)
    , m_LightImge(CreateRoundImage(200, 0X001286FF))
{}

void DrawArea::draw()
{
    BaseArea::draw();

    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ClearBackground()){ Clear(); }

    extern EditorMap g_EditorMap;
    if(g_EditorMap.Valid()){
        DrawTile();
        DrawAttributeGrid();
        DrawObject(true);
        DrawObject(false);

        DrawLight();
        DrawGrid();

        DrawDoneSelect();
        DrawTrySelect();
        DrawTextBox();
    }
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
                g_EditorMap.Cell(nX, nY).SelectConf.Ground = g_MainWindow->Deselect() ? false : true;
            }
        }
    };

    int nMX = m_MouseX + m_OffsetX - x();
    int nMY = m_MouseY + m_OffsetY - y();

    extern SelectSettingWindow *g_SelectSettingWindow;
    AttributeCoverOperation(nMX, nMY, g_SelectSettingWindow->AttributeSize(), fnSet);
}

void DrawArea::DrawTrySelectByTile()
{
    int nX = ((m_MouseX - x() + m_OffsetX) / (2 * SYS_MAPGRIDXP)) * 2;
    int nY = ((m_MouseY - y() + m_OffsetY) / (2 * SYS_MAPGRIDYP)) * 2;

    extern MainWindow *g_MainWindow;
    FillRectangle(
            SYS_MAPGRIDXP * nX - m_OffsetX,
            SYS_MAPGRIDYP * nY - m_OffsetY,
            SYS_MAPGRIDXP * 2,
            SYS_MAPGRIDYP * 2,
            g_MainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
}

void DrawArea::DrawDoneSelectByObject(bool bGround)
{
    int nX0 = m_OffsetX / SYS_MAPGRIDXP - SYS_OBJMAXW;
    int nY0 = m_OffsetY / SYS_MAPGRIDYP - SYS_OBJMAXH;

    int nX1 = (m_OffsetX + w()) / SYS_MAPGRIDXP + SYS_OBJMAXW;
    int nY1 = (m_OffsetY + h()) / SYS_MAPGRIDYP + SYS_OBJMAXH;

    for(int nX = nX0; nX < nX1; ++nX){
        for(int nY = nY0; nY < nY1; ++nY){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                extern EditorMap g_EditorMap;
                if(g_EditorMap.ValidC(nX, nY)){

                    auto rstObj = g_EditorMap.Object(nX, nY, nIndex);
                    if(true
                            && rstObj.Valid
                            && rstObj.Ground == bGround){

                        auto rstCell = g_EditorMap.Cell(nX, nY);
                        if(false
                                || ( bGround && rstCell.SelectConf.GroundObj)
                                || (!bGround && rstCell.SelectConf.OverGroundObj)){

                            auto nFileIndex  = (uint8_t )((rstObj.Image & 0X00FF0000) >> 16);
                            auto nImageIndex = (uint16_t)((rstObj.Image & 0X0000FFFF) >>  0);

                            if(auto pImage = RetrievePNG(nFileIndex, nImageIndex)){
                                int nW = pImage->w();
                                int nH = pImage->h();

                                int nStartX = nX * SYS_MAPGRIDXP - m_OffsetX;
                                int nStartY = nY * SYS_MAPGRIDYP - m_OffsetY + SYS_MAPGRIDYP - nH;

                                extern MainWindow *g_MainWindow;
                                FillRectangle(nStartX, nStartY, nW, nH, g_MainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
                            }
                        }
                    }
                }
            }
        }
    }
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
                            FillRectangle(nStartX, nStartY, nW, nH, g_MainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
                            DrawFloatObject(nX, nCurrY, (nIndex == 0) ? FOTYPE_OBJ0 : FOTYPE_OBJ1, m_MouseX - x(), m_MouseY - y());
                            return;
                        }
                    }
                }
            }
        }
    }
}

void DrawArea::DrawTrySelectBySingle()
{
    int nX = (m_MouseX - x() + m_OffsetX) / SYS_MAPGRIDXP;
    int nY = (m_MouseY - y() + m_OffsetY) / SYS_MAPGRIDYP;

    extern MainWindow *g_MainWindow;
    FillRectangle(
            SYS_MAPGRIDXP * nX - m_OffsetX,
            SYS_MAPGRIDYP * nY - m_OffsetY,
            SYS_MAPGRIDXP,
            SYS_MAPGRIDYP,
            g_MainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
}

void DrawArea::AddSelectByObject(bool bGround)
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
                            auto &rstCell = g_EditorMap.Cell(nX, nCurrY);
                            extern MainWindow *g_MainWindow;
                            if(bGround){
                                rstCell.SelectConf.GroundObj = !g_MainWindow->Deselect();
                            }else{
                                rstCell.SelectConf.OverGroundObj = !g_MainWindow->Deselect();
                            }
                            return;
                        }
                    }
                }
            }
        }
    }
}

void DrawArea::AddSelectBySingle()
{
    int nX = (m_MouseX - x() + m_OffsetX) / SYS_MAPGRIDXP;
    int nY = (m_MouseY - y() + m_OffsetY) / SYS_MAPGRIDYP;

    extern EditorMap g_EditorMap;
    if(g_EditorMap.ValidC(nX, nY)){
        extern MainWindow *g_MainWindow;
        g_EditorMap.Cell(nX, nY).SelectConf.Ground = (g_MainWindow->Deselect() ? false : true);
    }
}

void DrawArea::RhombusCoverOperation(int nMX, int nMY, int nSize, std::function<void(int, int)> fnOperation)
{
    // don't check boundary condition
    // since even center point is out of map
    // we can still select grids over map

    if(nSize > 0){
        
        // rhombus we have size as 1, 3, 5 as following
        // then for size as 2, 4, 6 we draw 3, 5, 7 instead
        //
        //             *
        //       *    ***
        //  *   ***  *****
        //       *    ***
        //             *

        int nResize = (nSize / 2) * 2 + 1;
        int nStartX = (nMX / SYS_MAPGRIDXP) - nResize / 2;
        int nStartY = (nMY / SYS_MAPGRIDYP) - nResize / 2;

        for(int nX = nStartX; nX < nStartX + nResize; ++nX){
            for(int nY = nStartY; nY < nStartY + nResize; ++nY){
                int nDX = nX - (nStartX + nResize / 2);
                int nDY = nY - (nStartY + nResize / 2);
                if(std::abs(nDX) + std::abs(nDY) <= nResize / 2){
                    fnOperation(nX, nY);
                }
            }
        }
    }
}

void DrawArea::DrawTrySelectByRhombus()
{
    auto fnDraw = [this](int nX, int nY)
    {
        extern MainWindow *g_MainWindow;
        FillMapGrid(nX, nY, 1, 1, g_MainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
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
            g_EditorMap.Cell(nX, nY).SelectConf.Ground = g_MainWindow->Deselect() ? false : true;
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

void DrawArea::DrawTrySelectByRectangle()
{
    auto fnDraw = [this](int nX, int nY)
    {
        extern MainWindow *g_MainWindow;
        FillMapGrid(nX, nY, 1, 1, g_MainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
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
        g_EditorMap.Cell(nX, nY).SelectConf.Ground = g_MainWindow->Deselect() ? false : true;
    };

    int nMX = m_MouseX + m_OffsetX - x();
    int nMY = m_MouseY + m_OffsetY - y();

    extern SelectSettingWindow *g_SelectSettingWindow;
    RectangleCoverOperation(nMX, nMY, g_SelectSettingWindow->RectangleSize(), fnSet);
}

void DrawArea::DrawTrySelectByAttribute()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->AttributeSize();

    if(nSize > 0){

        int nMX = m_MouseX + m_OffsetX - x();
        int nMY = m_MouseY + m_OffsetY - y();

        auto fnDraw = [this](int nX, int nY) -> void
        {
            extern EditorMap g_EditorMap;
            extern MainWindow *g_MainWindow;
            extern AttributeSelectWindow *g_AttributeSelectWindow;

            if(g_AttributeSelectWindow->TestLand(g_EditorMap.Cell(nX, nY).MakeLandU8())){
                FillMapGrid(nX, nY, 1, 1, g_MainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
            }
        };

        AttributeCoverOperation(nMX, nMY, nSize, fnDraw);
    }
}

void DrawArea::DrawDoneSelectByAttribute()
{
    int nX0 = m_OffsetX / SYS_MAPGRIDXP - 1;
    int nY0 = m_OffsetY / SYS_MAPGRIDYP - 1;
    int nX1 = (m_OffsetX + w()) / SYS_MAPGRIDXP + 1;
    int nY1 = (m_OffsetY + h()) / SYS_MAPGRIDYP + 1;

    for(int nX = nX0; nX < nX1; ++nX){
        for(int nY = nY0; nY < nY1; ++nY){
            extern EditorMap g_EditorMap;
            if(g_EditorMap.ValidC(nX, nY)){
                extern MainWindow *g_MainWindow;
                if(g_EditorMap.Cell(nX, nY).SelectConf.Attribute == !g_MainWindow->Reversed()){
                    FillMapGrid(nX, nY, 1, 1, g_MainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
                }
            }
        }
    }
}

void DrawArea::DrawDoneSelect()
{
    DrawDoneSelectByAttribute();
    DrawDoneSelectByObject(true);
    DrawDoneSelectByObject(false);
}

void DrawArea::DrawTrySelect()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->EnableSelect()){
        auto nColor = fl_color();
        fl_color(FL_RED);

        if(g_MainWindow->SelectByTile()){
            DrawTrySelectByTile();
        }

        if(g_MainWindow->SelectByObjectGround()){
            DrawSelectByObjectGround(true);
        }

        if(g_MainWindow->SelectByObjectOverGround()){
            DrawSelectByObjectGround(false);
        }

        if(g_MainWindow->SelectBySingle()){
            DrawTrySelectBySingle();
        }

        if(g_MainWindow->SelectByRhombus()){
            DrawTrySelectByRhombus();
        }

        if(g_MainWindow->SelectByRectangle()){
            DrawTrySelectByRectangle();
        }

        if(g_MainWindow->SelectByAttribute()){
            DrawTrySelectByAttribute();
        }

        fl_color(nColor);
    }
}

void DrawArea::DrawTextBox()
{
    FillRectangle(0, 0, 250, 150, 0XC0000000);
    PushColor(FL_RED);

    int nY = 20;

    DrawText(10, nY, "OffsetX: %d %d", m_OffsetX / SYS_MAPGRIDXP, m_OffsetX); nY += 20;
    DrawText(10, nY, "OffsetY: %d %d", m_OffsetY / SYS_MAPGRIDYP, m_OffsetY); nY += 20;

    int nMouseX = (std::max)(0, m_MouseX + m_OffsetX - x());
    int nMouseY = (std::max)(0, m_MouseY + m_OffsetY - y());

    DrawText(10, nY, "MouseMX: %d %d", nMouseX / SYS_MAPGRIDXP, nMouseX); nY += 20;
    DrawText(10, nY, "MouseMY: %d %d", nMouseY / SYS_MAPGRIDYP, nMouseY); nY += 20;

    PopColor();
}

void DrawArea::DrawObject(bool bGround)
{
    extern MainWindow *g_MainWindow;
    auto nColor = fl_color();
    if(false
            || ( bGround && g_MainWindow->ShowGroundObject())
            || (!bGround && g_MainWindow->ShowOverGroundObject())){

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
            auto pImage = RetrievePNG(nFileIndex, nImageIndex);
            if(pImage){
                // TODO: till now I still have no idea of this offset (-200, -157), I think maybe it's
                //       nothing and I can just ignore it
                //
                // int nStartX = nXCnt * SYS_MAPGRIDXP - 200;
                // int nStartY = nYCnt * SYS_MAPGRIDYP - 157 + SYS_MAPGRIDYP - pImage->h();

                int nStartX = -1;
                int nStartY = -1;

                if(false
                        || (pImage->w() == 1 * SYS_MAPGRIDXP && pImage->h() == 1 * SYS_MAPGRIDYP)
                        || (pImage->w() == 2 * SYS_MAPGRIDXP && pImage->h() == 2 * SYS_MAPGRIDYP)){
                    nStartX = nXCnt * SYS_MAPGRIDXP - m_OffsetX;
                    nStartY = nYCnt * SYS_MAPGRIDYP - m_OffsetY;
                }else{
                    nStartX = nXCnt * SYS_MAPGRIDXP - m_OffsetX;
                    nStartY = nYCnt * SYS_MAPGRIDYP + SYS_MAPGRIDYP - pImage->h() - m_OffsetY;
                }

                DrawImage(pImage, nStartX, nStartY);

                extern MainWindow *g_MainWindow;
                if(bGround){
                    if(g_MainWindow->ShowGroundObjectLine()){
                        DrawRectangle(nStartX, nStartY, pImage->w(), pImage->h());
                    }
                }else{
                    // ok we're drawing over-ground object
                    if(g_MainWindow->ShowOverGroundObjectLine()){
                        DrawRectangle(nStartX, nStartY, pImage->w(), pImage->h());
                    }
                }
            }
        };

        extern EditorMap g_EditorMap;
        g_EditorMap.DrawObject(m_OffsetX / SYS_MAPGRIDXP - 10, m_OffsetY / SYS_MAPGRIDYP - 20, w() / SYS_MAPGRIDXP + 20, h() / SYS_MAPGRIDYP + 40, bGround, fnDrawObj, fnDrawExt);

        fl_color(nColor);
    }
}

void DrawArea::DrawAttributeGrid()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ShowAttributeGridLine()){
        PushColor(FL_MAGENTA);  

        int nX0 = m_OffsetX / SYS_MAPGRIDXP - 1;
        int nY0 = m_OffsetY / SYS_MAPGRIDYP - 1;
        int nX1 = (m_OffsetX + w()) / SYS_MAPGRIDXP + 1;
        int nY1 = (m_OffsetY + h()) / SYS_MAPGRIDYP + 1;

        for(int nCX = nX0; nCX < nX1; ++nCX){
            for(int nCY = nY0; nCY < nY1; ++nCY){
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
        PopColor();
    }
}

void DrawArea::DrawGrid()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ShowGridLine()){
        PushColor(FL_MAGENTA);
        for(int nCX = m_OffsetX / SYS_MAPGRIDXP - 1; nCX < (m_OffsetX + w()) / SYS_MAPGRIDXP + 1; ++nCX){
            DrawLine(nCX * SYS_MAPGRIDXP - m_OffsetX, 0, nCX * SYS_MAPGRIDXP - m_OffsetX, h());
        }

        for(int nCY = m_OffsetY / SYS_MAPGRIDYP - 1; nCY < (m_OffsetY + h()) / SYS_MAPGRIDYP + 1; ++nCY){
            DrawLine(0, nCY * SYS_MAPGRIDYP - m_OffsetY, w(), nCY * SYS_MAPGRIDYP - m_OffsetY);
        }
        PopColor();
    }
}

Fl_Image *DrawArea::RetrievePNG(uint8_t nFileIndex, uint16_t nImageIndex)
{
    extern ImageDB    g_ImageDB;
    extern ImageCache g_ImageCache;
    auto pImage = g_ImageCache.Retrieve(nFileIndex, nImageIndex);
    if(pImage == nullptr){
        if(g_ImageDB.Valid(nFileIndex, nImageIndex)){
            int nW = g_ImageDB.FastW(nFileIndex);
            int nH = g_ImageDB.FastH(nFileIndex);
            g_ImageCache.Register(nFileIndex, nImageIndex, g_ImageDB.FastDecode(nFileIndex, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF), nW, nH);
            pImage = g_ImageCache.Retrieve(nFileIndex, nImageIndex);
        }
    }
    return pImage;
}

void DrawArea::DrawTile()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ShowTile()){
        PushColor(FL_RED);
        auto fnDraw = [this](uint8_t nFileIndex, uint16_t nImageIndex, int nX, int nY)
        {
            int nStartX = nX * SYS_MAPGRIDXP - m_OffsetX;
            int nStartY = nY * SYS_MAPGRIDYP - m_OffsetY;
            if(auto pImage = RetrievePNG(nFileIndex, nImageIndex)){
                DrawImage(pImage, nStartX, nStartY);
                extern MainWindow *g_MainWindow;
                if(g_MainWindow->ShowTileLine()){
                    DrawRectangle(nStartX, nStartY, pImage->w(), pImage->h());
                }
            }
        };

        int nX = m_OffsetX / SYS_MAPGRIDXP - 1;
        int nY = m_OffsetY / SYS_MAPGRIDYP - 1;
        int nW = w() / SYS_MAPGRIDXP + 3;
        int nH = h() / SYS_MAPGRIDYP + 8;

        extern EditorMap g_EditorMap;
        g_EditorMap.DrawTile(nX, nY, nW, nH, fnDraw);
        PopColor();
    }
}

void DrawArea::SetOffset(int nX, bool bRelativeX, int nY, bool bRelativeY)
{
    extern EditorMap g_EditorMap;
    if(g_EditorMap.Valid()){
        if(bRelativeX){
            m_OffsetX += nX;
        }else{
            m_OffsetX = nX;
        }
        m_OffsetX = (std::max)(m_OffsetX, 0);
        m_OffsetX = (std::min)(m_OffsetX, (std::max)(0, SYS_MAPGRIDXP * g_EditorMap.W() - w()));

        if(bRelativeY){
            m_OffsetY += nY;
        }else{
            m_OffsetY = nY;
        }
        m_OffsetY = (std::max)(m_OffsetY, 0);
        m_OffsetY = (std::min)(m_OffsetY, (std::max)(0, SYS_MAPGRIDYP * g_EditorMap.H() - h()));
    }
}

int DrawArea::handle(int nEvent)
{
    auto nRet = BaseArea::handle(nEvent);

    // can't find resize event
    // put it here as a hack and check it every time
    {
        extern MainWindow *g_MainWindow;
        g_MainWindow->CheckScrollBar();
    }

    extern EditorMap g_EditorMap;
    if(g_EditorMap.Valid()){

        int nMouseX = m_MouseX;
        int nMouseY = m_MouseY;
        m_MouseX    = Fl::event_x();
        m_MouseY    = Fl::event_y();

        switch(nEvent){
            case FL_FOCUS:
            case FL_UNFOCUS:
                {
                    nRet = 1;
                    break;
                }
            case FL_KEYDOWN:
                {
                    int nDX = 0;
                    int nDY = 0;
                    switch(Fl::event_key()){
                        case FL_Up   : { nDY = -1; break; }
                        case FL_Down : { nDY =  1; break; }
                        case FL_Left : { nDX = -1; break; }
                        case FL_Right: { nDX =  1; break; }
                        default      : {           break; }
                    }

                    SetOffset(nDX * SYS_MAPGRIDXP, true, nDY * SYS_MAPGRIDYP, true);
                    SetScrollBar();
                    break;
                }
            case FL_MOUSEWHEEL:
                {
                    int nDX = Fl::event_dx() * SYS_MAPGRIDXP;
                    int nDY = Fl::event_dy() * SYS_MAPGRIDYP;

                    SetOffset(nDX, true, nDY, true);
                    SetScrollBar();

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
    if(SYS_MAPGRIDXP * g_EditorMap.W() > w()){
        fXP = m_OffsetX * 1.0 / (SYS_MAPGRIDXP * g_EditorMap.W() - w());
    }

    if(SYS_MAPGRIDYP * g_EditorMap.H() > h()){
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

    AddSelectByObject(false);
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
    if(g_MainWindow->ShowLight()){
        auto fnDrawLight = [this](int nX, int nY) -> void
        {
            DrawImage(m_LightImge.get(),
                    nX * SYS_MAPGRIDXP - m_OffsetX + SYS_MAPGRIDXP / 2 - (m_LightImge->w() - 1) / 2,
                    nY * SYS_MAPGRIDYP - m_OffsetY + SYS_MAPGRIDYP / 2 - (m_LightImge->h() - 1) / 2);

            extern MainWindow *g_MainWindow;
            if(g_MainWindow->ShowLightLine()){
                PushColor(FL_RED);
                DrawCircle(nX * SYS_MAPGRIDXP - m_OffsetX, nY * SYS_MAPGRIDYP - m_OffsetY, 10);
                PopColor();
            }
        };

        int nX = m_OffsetX / SYS_MAPGRIDXP - 1;
        int nY = m_OffsetY / SYS_MAPGRIDYP - 1;
        int nW = w() / SYS_MAPGRIDXP + 3;
        int nH = h() / SYS_MAPGRIDYP + 8;

        extern EditorMap g_EditorMap;
        g_EditorMap.DrawLight(nX, nY, nW, nH, fnDrawLight);
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
                    nTextBoxH = 200;

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
                    nTextBoxH = 200;

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

            FillRectangle(nWinX, nWinY, nWinW, nWinH, 0X80000000);
            DrawImage(pImage, nImageX, nImageY);

            // draw boundary for window
            PushColor(FL_YELLOW);
            DrawRectangle(nWinX, nWinY, nWinW, nWinH);
            PopColor();

            // draw boundary for image
            PushColor(FL_MAGENTA);
            DrawRectangle(nImageX, nImageY, pImage->w(), pImage->h());
            PopColor();

            // draw textbox
            // after we allocated textbox
            // we have small offset to start inside it

            PushColor(FL_BLUE);
            DrawRectangle(nTextBoxX, nTextBoxY, nTextBoxW, nTextBoxH);
            PopColor();

            int nTextOffX = 20;
            int nTextOffY = 20;

            switch(nFOType){
                case FOTYPE_TILE:
                    {
                        PushColor(FL_RED);

                        int nTextStartX = nTextBoxX + nTextOffX;
                        int nTextStartY = nTextBoxY + nTextOffY;

                        DrawText(nTextStartX, nTextStartY, "     Tile");
                        nTextOffY += 20;

                        DrawText(nTextStartX, nTextStartY, "Index0 : %d", (int)(nFileIndex));
                        nTextOffY += 20;

                        DrawText(nTextStartX, nTextStartY, "Index1 : %d", (int)(nImageIndex));
                        nTextOffY += 20;

                        extern ImageDB g_ImageDB;
                        DrawText(nTextStartX, nTextStartY, "DBName : %s", g_ImageDB.DBName(nFileIndex));
                        nTextOffY += 20;

                        PopColor();
                        break;
                    }
                case FOTYPE_OBJ0:
                case FOTYPE_OBJ1:
                    {
                        PushColor(FL_RED);

                        int nTextStartX = nTextBoxX + nTextOffX;
                        int nTextStartY = nTextBoxY + nTextOffY;

                        DrawText(nTextStartX, nTextStartY, "    Obj[%d]", (nFOType == FOTYPE_OBJ0) ? 0 : 1);
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "Index0 : %d", (int)(nFileIndex));
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "Index1 : %d", (int)(nImageIndex));
                        nTextStartY += 20;

                        extern ImageDB g_ImageDB;
                        DrawText(nTextStartX, nTextStartY, "DBName : %s", g_ImageDB.DBName(nFileIndex));
                        nTextStartY += 20;

                        auto &rstObject = g_EditorMap.Object(nX, nY, (nFOType == FOTYPE_OBJ0) ? 0 : 1);
                        DrawText(nTextStartX, nTextStartY, "ABlend : %s", rstObject.Alpha ? "yes" : "no");
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "Animat : %s", rstObject.Animated ? "yes" : "no");
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "AniTyp : %d", rstObject.AniType);
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "AniCnt : %d", rstObject.AniCount);
                        nTextStartY += 20;

                        PopColor();
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

void DrawArea::FillMapGrid(int nX, int nY, int nW, int nH, uint32_t nARGB)
{
    int nPX = nX * SYS_MAPGRIDXP - m_OffsetX;
    int nPY = nY * SYS_MAPGRIDYP - m_OffsetY;

    int nPW = nW * SYS_MAPGRIDXP;
    int nPH = nH * SYS_MAPGRIDYP;

    FillRectangle(nPX, nPY, nPW, nPH, nARGB);
}

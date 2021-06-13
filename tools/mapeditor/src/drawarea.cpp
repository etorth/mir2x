/*
 * =====================================================================================
 *
 *       Filename: drawarea.cpp
 *        Created: 07/26/2017 04:27:57
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
#include "mathf.hpp"
#include "editormap.hpp"
#include "colorf.hpp"
#include "animation.hpp"
#include "mainwindow.hpp"
#include "imagecache.hpp"
#include "animationdb.hpp"
#include "animationdraw.hpp"
#include "animationselectwindow.hpp"
#include "attributeselectwindow.hpp"

extern ImageDB *g_imageDB;
extern EditorMap g_editorMap;
extern ImageCache g_imageCache;

extern MainWindow *g_mainWindow;
extern SelectSettingWindow *g_selectSettingWindow;
extern AttributeSelectWindow *g_attributeSelectWindow;

std::tuple<int, int> DrawArea::offset() const
{
    if(!g_editorMap.Valid()){
        return {0, 0};
    }

    const auto [xpCount, ypCount] = getScrollPixelCount();
    const auto [xratio, yratio] = g_mainWindow->getScrollBarValue();

    return
    {
        to_d(std::lround(xratio * xpCount)),
        to_d(std::lround(yratio * ypCount)),
    };
}

void DrawArea::draw()
{
    BaseArea::draw();
    if(g_mainWindow->ClearBackground()){
        Clear();
    }

    if(g_editorMap.Valid()){
        DrawTile();
        DrawObject(true);
        DrawObject(false);
        DrawAttributeGrid();

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
        if(g_editorMap.ValidC(nX, nY)){
            if(g_attributeSelectWindow->TestLand(g_editorMap.Cell(nX, nY).MakeLandU8())){
                g_editorMap.Cell(nX, nY).SelectConf.Ground = g_mainWindow->Deselect() ? false : true;
            }
        }
    };

    const auto [offsetX, offsetY] = offset();

    int nMX = m_mouseX + offsetX - x();
    int nMY = m_mouseY + offsetY - y();

    AttributeCoverOperation(nMX, nMY, g_selectSettingWindow->AttributeSize(), fnSet);
}

void DrawArea::DrawDoneSelectByTile()
{
    const auto [offsetX, offsetY] = offset();

    int nX0 = offsetX / SYS_MAPGRIDXP - SYS_OBJMAXW;
    int nY0 = offsetY / SYS_MAPGRIDYP - SYS_OBJMAXH;

    int nX1 = (offsetX + w()) / SYS_MAPGRIDXP + SYS_OBJMAXW;
    int nY1 = (offsetY + h()) / SYS_MAPGRIDYP + SYS_OBJMAXH;

    for(int nX = nX0; nX < nX1; ++nX){
        for(int nY = nY0; nY < nY1; ++nY){
            if(true
                    && !(nX % 2)
                    && !(nY % 2)
                    &&  (g_editorMap.ValidC(nX, nY))){

                auto &rstTile = g_editorMap.Tile(nX, nY);
                if(true
                        && rstTile.Valid
                        && rstTile.SelectConf.Tile){

                    int nStartX = nX * SYS_MAPGRIDXP - offsetX;
                    int nStartY = nY * SYS_MAPGRIDYP - offsetY;

                    FillRectangle(nStartX, nStartY, SYS_MAPGRIDXP * 2, SYS_MAPGRIDYP * 2, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
                }
            }
        }
    }
}

void DrawArea::DrawDoneSelectByObject(bool bGround)
{
    const auto [offsetX, offsetY] = offset();

    int nX0 = offsetX / SYS_MAPGRIDXP - SYS_OBJMAXW;
    int nY0 = offsetY / SYS_MAPGRIDYP - SYS_OBJMAXH;

    int nX1 = (offsetX + w()) / SYS_MAPGRIDXP + SYS_OBJMAXW;
    int nY1 = (offsetY + h()) / SYS_MAPGRIDYP + SYS_OBJMAXH;

    for(int nX = nX0; nX < nX1; ++nX){
        for(int nY = nY0; nY < nY1; ++nY){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                if(g_editorMap.ValidC(nX, nY)){

                    auto rstObj = g_editorMap.Object(nX, nY, nIndex);
                    if(true
                            && rstObj.Valid
                            && rstObj.Ground == bGround){

                        auto rstCell = g_editorMap.Cell(nX, nY);
                        if(false
                                || ( bGround && rstCell.SelectConf.GroundObj)
                                || (!bGround && rstCell.SelectConf.OverGroundObj)){

                            auto nFileIndex  = (uint8_t )((rstObj.Image & 0X00FF0000) >> 16);
                            auto nImageIndex = to_u16((rstObj.Image & 0X0000FFFF) >>  0);

                            if(auto pImage = RetrievePNG(nFileIndex, nImageIndex)){
                                int nW = pImage->w();
                                int nH = pImage->h();

                                int nStartX = nX * SYS_MAPGRIDXP - offsetX;
                                int nStartY = nY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - nH;

                                FillRectangle(nStartX, nStartY, nW, nH, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
                            }
                        }
                    }
                }
            }
        }
    }
}

void DrawArea::DrawTrySelectByTile()
{
    const auto [offsetX, offsetY] = offset();

    int nMouseXOnMap = m_mouseX - x() + offsetX;
    int nMouseYOnMap = m_mouseY - y() + offsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    if(true
            && g_editorMap.ValidC(nX, nY)
            && g_editorMap.Tile(nX, nY).Valid){

        int nStartX = (nX / 2) * 2 * SYS_MAPGRIDXP - offsetX;
        int nStartY = (nY / 2) * 2 * SYS_MAPGRIDYP - offsetY;

        FillRectangle(nStartX, nStartY, SYS_MAPGRIDXP * 2,  SYS_MAPGRIDYP * 2, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
        DrawFloatObject((nX / 2) * 2, (nY / 2) * 2, FOTYPE_TILE, m_mouseX - x(), m_mouseY - y());
    }
}

void DrawArea::DrawSelectByObjectGround(bool bGround)
{
    const auto [offsetX, offsetY] = offset();

    int nMouseXOnMap = m_mouseX - x() + offsetX;
    int nMouseYOnMap = m_mouseY - y() + offsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    for(int nCurrY = nY - 1; nCurrY < g_editorMap.H(); ++nCurrY){
        if(g_editorMap.ValidC(nX, nCurrY)){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                auto &rstObject = g_editorMap.Object(nX, nCurrY, nIndex);
                if(true
                        && rstObject.Valid
                        && rstObject.Ground == bGround){

                    auto nFileIndex  = (uint8_t )((rstObject.Image & 0X00FF0000) >> 16);
                    auto nImageIndex = to_u16((rstObject.Image & 0X0000FFFF) >>  0);

                    if(auto pImage = RetrievePNG(nFileIndex, nImageIndex)){
                        int nW = pImage->w();
                        int nH = pImage->h();

                        int nStartX = nX     * SYS_MAPGRIDXP - offsetX;
                        int nStartY = nCurrY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - nH;

                        if(mathf::pointInRectangle(nMouseXOnMap - offsetX, nMouseYOnMap - offsetY, nStartX, nStartY, nW, nH)){
                            FillRectangle(nStartX, nStartY, nW, nH, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
                            DrawFloatObject(nX, nCurrY, (nIndex == 0) ? FOTYPE_OBJ0 : FOTYPE_OBJ1, m_mouseX - x(), m_mouseY - y());
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
    const auto [offsetX, offsetY] = offset();

    int nX = (m_mouseX - x() + offsetX) / SYS_MAPGRIDXP;
    int nY = (m_mouseY - y() + offsetY) / SYS_MAPGRIDYP;

    FillRectangle(
            SYS_MAPGRIDXP * nX - offsetX,
            SYS_MAPGRIDYP * nY - offsetY,
            SYS_MAPGRIDXP,
            SYS_MAPGRIDYP,
            g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
}

void DrawArea::AddSelectByTile()
{
    const auto [offsetX, offsetY] = offset();

    int nMouseXOnMap = m_mouseX - x() + offsetX;
    int nMouseYOnMap = m_mouseY - y() + offsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    if(true
            && g_editorMap.ValidC(nX, nY)
            && g_editorMap.Tile(nX, nY).Valid){
        g_editorMap.Tile(nX, nY).SelectConf.Tile = true;
    }
}

void DrawArea::AddSelectByObject(bool bGround)
{
    const auto [offsetX, offsetY] = offset();

    int nMouseXOnMap = m_mouseX - x() + offsetX;
    int nMouseYOnMap = m_mouseY - y() + offsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    for(int nCurrY = nY - 1; nCurrY < g_editorMap.H(); ++nCurrY){
        if(g_editorMap.ValidC(nX, nCurrY)){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                auto &rstObject = g_editorMap.Object(nX, nCurrY, nIndex);
                if(true
                        && rstObject.Valid
                        && rstObject.Ground == bGround){

                    auto nFileIndex  = (uint8_t )((rstObject.Image & 0X00FF0000) >> 16);
                    auto nImageIndex = to_u16((rstObject.Image & 0X0000FFFF) >>  0);

                    if(auto pImage = RetrievePNG(nFileIndex, nImageIndex)){
                        int nW = pImage->w();
                        int nH = pImage->h();

                        int nStartX = nX     * SYS_MAPGRIDXP - offsetX;
                        int nStartY = nCurrY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - nH;

                        if(mathf::pointInRectangle(nMouseXOnMap - offsetX, nMouseYOnMap - offsetY, nStartX, nStartY, nW, nH)){
                            auto &rstCell = g_editorMap.Cell(nX, nCurrY);
                            if(bGround){
                                rstCell.SelectConf.GroundObj = !g_mainWindow->Deselect();
                            }else{
                                rstCell.SelectConf.OverGroundObj = !g_mainWindow->Deselect();
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
    const auto [offsetX, offsetY] = offset();

    int nX = (m_mouseX - x() + offsetX) / SYS_MAPGRIDXP;
    int nY = (m_mouseY - y() + offsetY) / SYS_MAPGRIDYP;

    if(g_editorMap.ValidC(nX, nY)){
        g_editorMap.Cell(nX, nY).SelectConf.Ground = (g_mainWindow->Deselect() ? false : true);
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
        FillMapGrid(nX, nY, 1, 1, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
    };

    const auto [offsetX, offsetY] = offset();

    int nMX = m_mouseX + offsetX - x();
    int nMY = m_mouseY + offsetY - y();

    RhombusCoverOperation(nMX, nMY, g_selectSettingWindow->RhombusSize(), fnDraw);
}

void DrawArea::AddSelectByRhombus()
{
    auto fnSet = [](int nX, int nY)
    {
        if(g_editorMap.ValidC(nX, nY)){
            g_editorMap.Cell(nX, nY).SelectConf.Ground = g_mainWindow->Deselect() ? false : true;
        }
    };

    const auto [offsetX, offsetY] = offset();

    int nMX = m_mouseX + offsetX - x();
    int nMY = m_mouseY + offsetY - y();

    RhombusCoverOperation(nMX, nMY, g_selectSettingWindow->RhombusSize(), fnSet);
}

void DrawArea::RectangleCoverOperation(int nMouseXOnMap, int nMouseYOnMap, int nSize, std::function<void(int, int)> fnOperation)
{
    if(nSize > 0){
        int nMX = nMouseXOnMap / SYS_MAPGRIDXP - nSize / 2;
        int nMY = nMouseYOnMap / SYS_MAPGRIDYP - nSize / 2;

        for(int nX = 0; nX < nSize; ++nX){
            for(int nY = 0; nY < nSize; ++nY){
                if(g_editorMap.ValidC(nX + nMX, nY + nMY)){
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
        FillMapGrid(nX, nY, 1, 1, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
    };

    const auto [offsetX, offsetY] = offset();

    int nMX = m_mouseX + offsetX - x();
    int nMY = m_mouseY + offsetY - y();

    RectangleCoverOperation(nMX, nMY, g_selectSettingWindow->RectangleSize(), fnDraw);
}

void DrawArea::AddSelectByRectangle()
{
    auto fnSet = [](int nX,  int nY)
    {
        g_editorMap.Cell(nX, nY).SelectConf.Ground = g_mainWindow->Deselect() ? false : true;
    };

    const auto [offsetX, offsetY] = offset();

    int nMX = m_mouseX + offsetX - x();
    int nMY = m_mouseY + offsetY - y();

    RectangleCoverOperation(nMX, nMY, g_selectSettingWindow->RectangleSize(), fnSet);
}

void DrawArea::DrawTrySelectByAttribute()
{
    int nSize = g_selectSettingWindow->AttributeSize();

    if(nSize > 0){

        const auto [offsetX, offsetY] = offset();

        int nMX = m_mouseX + offsetX - x();
        int nMY = m_mouseY + offsetY - y();

        auto fnDraw = [this](int nX, int nY) -> void
        {

            if(g_attributeSelectWindow->TestLand(g_editorMap.Cell(nX, nY).MakeLandU8())){
                FillMapGrid(nX, nY, 1, 1, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
            }
        };

        AttributeCoverOperation(nMX, nMY, nSize, fnDraw);
    }
}

void DrawArea::DrawDoneSelectByAttribute()
{
    const auto [offsetX, offsetY] = offset();

    int nX0 = offsetX / SYS_MAPGRIDXP - 1;
    int nY0 = offsetY / SYS_MAPGRIDYP - 1;
    int nX1 = (offsetX + w()) / SYS_MAPGRIDXP + 1;
    int nY1 = (offsetY + h()) / SYS_MAPGRIDYP + 1;

    for(int nX = nX0; nX < nX1; ++nX){
        for(int nY = nY0; nY < nY1; ++nY){
            if(g_editorMap.ValidC(nX, nY)){
                if(g_editorMap.Cell(nX, nY).SelectConf.Attribute == !g_mainWindow->Reversed()){
                    FillMapGrid(nX, nY, 1, 1, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
                }
            }
        }
    }
}

void DrawArea::DrawDoneSelect()
{
    DrawDoneSelectByTile();
    DrawDoneSelectByAttribute();
    DrawDoneSelectByObject(true);
    DrawDoneSelectByObject(false);
}

void DrawArea::DrawTrySelect()
{
    if(g_mainWindow->EnableSelect()){
        auto nColor = fl_color();
        fl_color(FL_RED);

        if(g_mainWindow->SelectByTile()){
            DrawTrySelectByTile();
        }

        if(g_mainWindow->SelectByObject(true)){
            DrawSelectByObjectGround(true);
        }

        if(g_mainWindow->SelectByObject(false)){
            DrawSelectByObjectGround(false);
        }

        if(g_mainWindow->SelectBySingle()){
            DrawTrySelectBySingle();
        }

        if(g_mainWindow->SelectByRhombus()){
            DrawTrySelectByRhombus();
        }

        if(g_mainWindow->SelectByRectangle()){
            DrawTrySelectByRectangle();
        }

        if(g_mainWindow->SelectByAttribute()){
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

    const auto [offsetX, offsetY] = offset();

    DrawText(10, nY, "OffsetX: %d %d", offsetX / SYS_MAPGRIDXP, offsetX); nY += 20;
    DrawText(10, nY, "OffsetY: %d %d", offsetY / SYS_MAPGRIDYP, offsetY); nY += 20;

    int nMouseX = (std::max<int>)(0, m_mouseX + offsetX - x());
    int nMouseY = (std::max<int>)(0, m_mouseY + offsetY - y());

    DrawText(10, nY, "MouseMX: %d %d", nMouseX / SYS_MAPGRIDXP, nMouseX); nY += 20;
    DrawText(10, nY, "MouseMY: %d %d", nMouseY / SYS_MAPGRIDYP, nMouseY); nY += 20;

    PopColor();
}

void DrawArea::DrawObject(bool bGround)
{
    auto nColor = fl_color();
    if(false
            || ( bGround && g_mainWindow->ShowObject(bGround))
            || (!bGround && g_mainWindow->ShowObject(bGround))){

        fl_color(bGround ? FL_BLUE : FL_GREEN);
        auto fnDrawExt = [this, bGround](int /* nXCnt */, int /* nYCnt */) -> void
        {
            // if(!bGround){
            //     if(g_mainWindow->EnableTest()){
            //
            //         if(g_animationDraw.MonsterID){
            //             auto &rstAnimation = g_animationDB.RetrieveAnimation(g_animationDraw.MonsterID);
            //             if(true
            //                     && g_animationDraw.X / SYS_MAPGRIDXP == nXCnt
            //                     && g_animationDraw.Y / SYS_MAPGRIDYP == nYCnt){
            //                 if(rstAnimation.ResetFrame(g_animationDraw.Action, g_animationDraw.Direction, g_animationDraw.Frame)){
            //                     auto fnDraw = [this](Fl_Shared_Image *pPNG, int nMapX, int nMapY) -> void
            //                     {
            //                         DrawImage(pPNG, nMapX - offsetX, nMapY - offsetY);
            //                     };
            //
            //                     // use R from the AnimationSelectWindow, rather than the copy in AnimationDraw
            //                     // this enables me to adjust R without reset the animation MonsterID
            //                     int nAnimationX = g_animationDraw.X - offsetX;
            //                     int nAnimationY = g_animationDraw.Y - offsetY;
            //                     int nAnimationR = g_animationSelectWindow->R();
            //
            //                     // draw funcitons take coordinates w.r.t the window rather than the widget
            //                     // fl_circle(x() + nAnimationX * 1.0, y() + nAnimationY * 1.0, nAnimationR * 1.0);
            //                     DrawImage(m_coverV[nAnimationR], nAnimationX - nAnimationR, nAnimationY - nAnimationR);
            //                     fl_circle(x() + nAnimationX * 1.0, y() + nAnimationY * 1.0, nAnimationR * 1.0);
            //                     rstAnimation.Draw(g_animationDraw.X, g_animationDraw.Y, fnDraw);
            //                 }
            //             }
            //         }
            //     }
            // }
        };

        const auto [offsetX, offsetY] = offset();
        auto fnDrawObj = [this, offsetX, offsetY, bGround](uint8_t nFileIndex, uint16_t nImageIndex, int nXCnt, int nYCnt) -> void
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
                    nStartX = nXCnt * SYS_MAPGRIDXP - offsetX;
                    nStartY = nYCnt * SYS_MAPGRIDYP - offsetY;
                }else{
                    nStartX = nXCnt * SYS_MAPGRIDXP - offsetX;
                    nStartY = nYCnt * SYS_MAPGRIDYP + SYS_MAPGRIDYP - pImage->h() - offsetY;
                }

                DrawImage(pImage, nStartX, nStartY);

                if(bGround){
                    if(g_mainWindow->ShowGroundObjectLine()){
                        DrawRectangle(nStartX, nStartY, pImage->w(), pImage->h());
                    }
                }else{
                    // ok we're drawing over-ground object
                    if(g_mainWindow->ShowOverGroundObjectLine()){
                        DrawRectangle(nStartX, nStartY, pImage->w(), pImage->h());
                    }
                }
            }
        };

        g_editorMap.DrawObject(offsetX / SYS_MAPGRIDXP - 10, offsetY / SYS_MAPGRIDYP - 20, w() / SYS_MAPGRIDXP + 20, h() / SYS_MAPGRIDYP + 40, bGround, fnDrawObj, fnDrawExt);

        fl_color(nColor);
    }
}

void DrawArea::DrawAttributeGrid()
{
    if(g_mainWindow->ShowAttributeGridLine()){
        PushColor(FL_MAGENTA);  

        const auto [offsetX, offsetY] = offset();

        int nX0 = offsetX / SYS_MAPGRIDXP - 1;
        int nY0 = offsetY / SYS_MAPGRIDYP - 1;
        int nX1 = (offsetX + w()) / SYS_MAPGRIDXP + 1;
        int nY1 = (offsetY + h()) / SYS_MAPGRIDYP + 1;

        for(int nCX = nX0; nCX < nX1; ++nCX){
            for(int nCY = nY0; nCY < nY1; ++nCY){
                if(g_editorMap.ValidC(nCX, nCY)){
                    if(g_attributeSelectWindow->TestLand(g_editorMap.Cell(nCX, nCY).MakeLandU8())){
                        int nPX = nCX * SYS_MAPGRIDXP - offsetX;
                        int nPY = nCY * SYS_MAPGRIDYP - offsetY;
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
    if(g_mainWindow->ShowGridLine()){
        PushColor(FL_MAGENTA);
        const auto [offsetX, offsetY] = offset();
        for(int nCX = offsetX / SYS_MAPGRIDXP - 1; nCX < (offsetX + w()) / SYS_MAPGRIDXP + 1; ++nCX){
            DrawLine(nCX * SYS_MAPGRIDXP - offsetX, 0, nCX * SYS_MAPGRIDXP - offsetX, h());
        }

        for(int nCY = offsetY / SYS_MAPGRIDYP - 1; nCY < (offsetY + h()) / SYS_MAPGRIDYP + 1; ++nCY){
            DrawLine(0, nCY * SYS_MAPGRIDYP - offsetY, w(), nCY * SYS_MAPGRIDYP - offsetY);
        }
        PopColor();
    }
}

Fl_Image *DrawArea::RetrievePNG(uint8_t nFileIndex, uint16_t nImageIndex)
{
    auto pImage = g_imageCache.Retrieve(nFileIndex, nImageIndex);
    if(pImage == nullptr){
        if(const auto [imgBuf, imgWidth, imgHeight] = g_imageDB->decode(nFileIndex, nImageIndex, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF); imgBuf){
            g_imageCache.Register(nFileIndex, nImageIndex, imgBuf, imgWidth, imgHeight);
            pImage = g_imageCache.Retrieve(nFileIndex, nImageIndex);
        }
    }
    return pImage;
}

void DrawArea::DrawTile()
{
    if(g_mainWindow->ShowTile()){
        PushColor(FL_RED);
        const auto [offsetX, offsetY] = offset();
        auto fnDraw = [offsetX, offsetY, this](uint8_t nFileIndex, uint16_t nImageIndex, int nX, int nY)
        {
            int nStartX = nX * SYS_MAPGRIDXP - offsetX;
            int nStartY = nY * SYS_MAPGRIDYP - offsetY;
            if(auto pImage = RetrievePNG(nFileIndex, nImageIndex)){
                DrawImage(pImage, nStartX, nStartY);
                if(g_mainWindow->ShowTileLine()){
                    DrawRectangle(nStartX, nStartY, pImage->w(), pImage->h());
                }
            }
        };

        int nX = offsetX / SYS_MAPGRIDXP - 1;
        int nY = offsetY / SYS_MAPGRIDYP - 1;
        int nW = w() / SYS_MAPGRIDXP + 3;
        int nH = h() / SYS_MAPGRIDYP + 8;

        g_editorMap.DrawTile(nX, nY, nW, nH, fnDraw);
        PopColor();
    }
}

int DrawArea::handle(int nEvent)
{
    auto nRet = BaseArea::handle(nEvent);

    // can't find resize event
    // put it here as a hack and check it every time
    g_mainWindow->checkScrollBar();

    if(g_editorMap.Valid()){

        const int lastMouseX = m_mouseX;
        const int lastMouseY = m_mouseY;
        m_mouseX = Fl::event_x();
        m_mouseY = Fl::event_y();

        switch(nEvent){
            case FL_FOCUS:
            case FL_UNFOCUS:
                {
                    nRet = 1;
                    break;
                }
            case FL_KEYDOWN:
                {
                    float dxratio = 0.0;
                    float dyratio = 0.0;

                    const auto [dxRatioUnit, dyRatioUnit] = getScrollPixelRatio(SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                    switch(Fl::event_key()){
                        case FL_Up   : { dyratio = -dyRatioUnit; break; }
                        case FL_Down : { dyratio =  dyRatioUnit; break; }
                        case FL_Left : { dxratio = -dxRatioUnit; break; }
                        case FL_Right: { dxratio =  dxRatioUnit; break; }
                        default      : {                         break; }
                    }

                    g_mainWindow->addScrollBarValue(dxratio, dyratio);
                    break;
                }
            case FL_MOUSEWHEEL:
                {
                    const auto [dxratio, dyratio] = getScrollPixelRatio(Fl::event_dx() * SYS_MAPGRIDXP, Fl::event_dy() * SYS_MAPGRIDYP);
                    g_mainWindow->addScrollBarValue(dxratio, dyratio);
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
                    if(g_mainWindow->EnableSelect()){
                        AddSelect();
                    }
                    else if(g_mainWindow->EnableEdit()){
                        // TODO
                        //
                    }
                    else if(g_mainWindow->EnableTest()){
                        // // we are moving the animation to a proper place
                        // // if current position is invalid, then we permit any moving to get a valid
                        // // position, but if current position is valid, we reject any move request
                        // // which make the position invlaid again
                        //
                        // int nNewX = g_animationDraw.X + (m_mouseX - lastMouseX);
                        // int nNewY = g_animationDraw.Y + (m_mouseY - lastMouseY);
                        //
                        // if(CoverValid(g_animationDraw.X, g_animationDraw.Y, g_animationSelectWindow->R())){
                        //     if(CoverValid(nNewX, nNewY, g_animationSelectWindow->R())){
                        //         g_animationDraw.X = nNewX;
                        //         g_animationDraw.Y = nNewY;
                        //     }else{
                        //         // try to find a feasible internal point by binary search
                        //         int nX0 = g_animationDraw.X;
                        //         int nY0 = g_animationDraw.Y;
                        //         int nX1 = nNewX;
                        //         int nY1 = nNewY;
                        //         while((std::abs(nX1 - nX0) >= 2) || (std::abs(nY1 - nY0) >= 2)){
                        //             int nMidX = (nX0 + nX1) / 2;
                        //             int nMidY = (nY0 + nY1) / 2;
                        //
                        //             if(CoverValid(nMidX, nMidY, g_animationSelectWindow->R())){
                        //                 nX0 = nMidX;
                        //                 nY0 = nMidY;
                        //             }else{
                        //                 nX1 = nMidX;
                        //                 nY1 = nMidY;
                        //             }
                        //         }
                        //         g_animationDraw.X = nX0;
                        //         g_animationDraw.Y = nY0;
                        //     }
                        // }else{
                        //     // always allowed
                        //     g_animationDraw.X = nNewX;
                        //     g_animationDraw.Y = nNewY;
                        // }
                    }
                    else{
                        if(Fl::event_state() & FL_CTRL){
                            // bug of fltk here for windows, when some key is pressed, 
                            // event_x() and event_y() are incorrect!
                        }
                        else{
                            const auto [xpCount, ypCount] = getScrollPixelCount();
                            const float xratio = (xpCount > 0) ? (-1.0 * to_f(m_mouseX - lastMouseX) / xpCount) : 0.0;
                            const float yratio = (ypCount > 0) ? (-1.0 * to_f(m_mouseY - lastMouseY) / ypCount) : 0.0;
                            g_mainWindow->addScrollBarValue(xratio, yratio);
                        }
                    }

                    break;
                }
            case FL_PUSH:
                {
                    if(g_mainWindow->EnableSelect()){
                        AddSelect();
                    }

                    if(g_mainWindow->EnableEdit()){
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

    g_mainWindow->RedrawAll();

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
                auto nR2 = mathf::LDistance2<int>(nRX, nRY, 0, 0);
                if(nR2 <= nRadius * nRadius){
                    auto nA = to_u8(0X02 + std::lround(0X2F * (1.0 - 1.0 * nR2 / (nRadius * nRadius))));
                    stvBuf[nX + nY * nSize] = ((to_u32(nA)) << 24) | (nColor & 0X00FFFFFF);
                }
            }
        }
        return Fl_RGB_Image((uchar *)(&(stvBuf[0])), nSize, nSize, 4, 0).copy();
    }else{
        fl_alert("Invalid radius for CreateRoundImage(%d, 0X%08X)", nRadius, nColor);
        return nullptr;
    }
}

void DrawArea::ClearGroundSelect()
{
    if(g_editorMap.Valid()){
        g_editorMap.ClearGroundSelect();
    }
}

void DrawArea::AddSelect()
{
    if(g_mainWindow->SelectBySingle()){
        AddSelectBySingle();
    }

    if(g_mainWindow->SelectByRhombus()){
        AddSelectByRhombus();
    }

    if(g_mainWindow->SelectByRectangle()){
        AddSelectByRectangle();
    }

    if(g_mainWindow->SelectByAttribute()){
        AddSelectByAttribute();
    }
    
    if(g_mainWindow->SelectByTile()){
        AddSelectByTile();
    }

    if(g_mainWindow->SelectByObject(true)){
        AddSelectByObject(true);
    }

    if(g_mainWindow->SelectByObject(false)){
        AddSelectByObject(false);
    }
}

void DrawArea::AttributeCoverOperation(int nMouseXOnMap, int nMouseYOnMap, int nSize, std::function<void(int, int)> fnOperation)
{
    if(nSize> 0){
        int nMX = nMouseXOnMap / SYS_MAPGRIDXP;
        int nMY = nMouseYOnMap / SYS_MAPGRIDYP;

        for(int nX = nMX - (nSize / 2); nX < nMX + (nSize + 1) / 2; ++nX){
            for(int nY = nMY - (nSize / 2); nY < nMY + (nSize + 1) / 2; ++nY){
                if(g_editorMap.ValidC(nX, nY)){
                    fnOperation(nX, nY);
                }
            }
        }
    }
}

void DrawArea::DrawLight()
{
    if(g_mainWindow->ShowLight()){
        const auto [offsetX, offsetY] = offset();
        auto fnDrawLight = [offsetX, offsetY, this](int nX, int nY) -> void
        {
            DrawImage(m_lightImge.get(),
                    nX * SYS_MAPGRIDXP - offsetX + SYS_MAPGRIDXP / 2 - (m_lightImge->w() - 1) / 2,
                    nY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP / 2 - (m_lightImge->h() - 1) / 2);

            if(g_mainWindow->ShowLightLine()){
                PushColor(FL_RED);
                DrawCircle(nX * SYS_MAPGRIDXP - offsetX, nY * SYS_MAPGRIDYP - offsetY, 10);
                PopColor();
            }
        };

        int nX = offsetX / SYS_MAPGRIDXP - 1;
        int nY = offsetY / SYS_MAPGRIDYP - 1;
        int nW = w() / SYS_MAPGRIDXP + 3;
        int nH = h() / SYS_MAPGRIDYP + 8;

        g_editorMap.DrawLight(nX, nY, nW, nH, fnDrawLight);
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

    if(true
            && nFOType > FOTYPE_NONE
            && nFOType < FOTYPE_MAX
            && g_editorMap.ValidC(nX, nY)){

        uint8_t  nFileIndex  = 0;
        uint16_t nImageIndex = 0;

        Fl_Image *pImage = nullptr;
        switch(nFOType){
            case FOTYPE_TILE:
                {
                    if(true
                            && !(nX % 2)
                            && !(nY % 2)){

                        auto &rstTile = g_editorMap.Tile(nX, nY);
                        if(rstTile.Valid){
                            nFileIndex  = (uint8_t )((rstTile.Image & 0X00FF0000) >> 16);
                            nImageIndex = to_u16((rstTile.Image & 0X0000FFFF) >>  0);
                            pImage = RetrievePNG(nFileIndex, nImageIndex);
                        }
                    }
                    break;
                }
            case FOTYPE_OBJ0:
            case FOTYPE_OBJ1:
                {
                    auto &rstObject = g_editorMap.Object(nX, nY, (nFOType == FOTYPE_OBJ0) ? 0 : 1);
                    if(rstObject.Valid){
                        nFileIndex  = (uint8_t )((rstObject.Image & 0X00FF0000) >> 16);
                        nImageIndex = to_u16((rstObject.Image & 0X0000FFFF) >>  0);
                        pImage = RetrievePNG(nFileIndex, nImageIndex);
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }

        // setup text box size
        // based on different image type

        int nTextBoxW = -1;
        int nTextBoxH = -1;

        switch(nFOType){
            case FOTYPE_TILE:
                {
                    nTextBoxW = 100 + std::strlen(g_imageDB->dbName(nFileIndex)) * 10;
                    nTextBoxH = 150;
                    break;
                }
            case FOTYPE_OBJ0:
            case FOTYPE_OBJ1:
                {
                    nTextBoxW = 100 + std::strlen(g_imageDB->dbName(nFileIndex)) * 10;
                    nTextBoxH = 260;
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

            FillRectangle(nWinX, nWinY, nWinW, nWinH, 0XC0000000);
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
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "Index0 : %d", to_d(nFileIndex));
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "Index1 : %d", to_d(nImageIndex));
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "DBName : %s", g_imageDB->dbName(nFileIndex));
                        nTextStartY += 20;

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

                        DrawText(nTextStartX, nTextStartY, "Index0 : %d", to_d(nFileIndex));
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "Index1 : %d", to_d(nImageIndex));
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "DBName : %s", g_imageDB->dbName(nFileIndex));
                        nTextStartY += 20;

                        auto &rstObject = g_editorMap.Object(nX, nY, (nFOType == FOTYPE_OBJ0) ? 0 : 1);
                        DrawText(nTextStartX, nTextStartY, "ABlend : %s", rstObject.Alpha ? "yes" : "no");
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "Animat : %s", rstObject.Animated ? "yes" : "no");
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "AniTyp : %d", rstObject.AniType);
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "AniCnt : %d", rstObject.AniCount);
                        nTextStartY += 20;

                        const auto imgInfo = g_imageDB->setIndex(nFileIndex, nImageIndex);
                        DrawText(nTextStartX, nTextStartY, "OffseX : %d", to_d(imgInfo->px));
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "OffseY : %d", to_d(imgInfo->py));
                        nTextStartY += 20;

                        const auto &rstHeader = g_imageDB->getPackage(nFileIndex)->header();
                        auto nVersion = rstHeader.version;

                        DrawText(nTextStartX, nTextStartY, "Versio : %d", to_d(nVersion));
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
    const auto [offsetX, offsetY] = offset();

    int nPX = nX * SYS_MAPGRIDXP - offsetX;
    int nPY = nY * SYS_MAPGRIDYP - offsetY;

    int nPW = nW * SYS_MAPGRIDXP;
    int nPH = nH * SYS_MAPGRIDYP;

    FillRectangle(nPX, nPY, nPW, nPH, nARGB);
}

std::tuple<size_t, size_t> DrawArea::getScrollPixelCount() const
{
    if(!g_editorMap.Valid()){
        return {0, 0};
    }

    return
    {
        [this]() -> size_t
        {
            if(g_editorMap.W() * SYS_MAPGRIDXP > w()){
                return g_editorMap.W() * SYS_MAPGRIDXP - w();
            }
            return 0;
        }(),

        [this]() -> float
        {
            if(g_editorMap.H() * SYS_MAPGRIDYP > h()){
                return g_editorMap.H() * SYS_MAPGRIDYP - h();
            }
            return 0;
        }(),
    };
}

std::tuple<float, float> DrawArea::getScrollPixelRatio(int dx, int dy) const
{
    const auto [xpCount, ypCount] = getScrollPixelCount();
    return
    {
        (xpCount > 0) ? to_f(dx) / xpCount : 0.0,
        (ypCount > 0) ? to_f(dy) / ypCount : 0.0,
    };
}

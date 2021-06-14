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
#include "flwrapper.hpp"
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
    if(!g_editorMap.valid()){
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
    if(g_mainWindow->clearBackground()){
        clear();
    }

    if(!g_editorMap.valid()){
        return;
    }

    drawTile();

    drawObject(OBJD_GROUND);
    drawObject(OBJD_OVERGROUND0);
    drawObject(OBJD_OVERGROUND1);
    drawObject(OBJD_SKY);

    DrawAttributeGrid();

    DrawLight();
    DrawGrid();

    DrawDoneSelect();
    DrawTrySelect();
    DrawTextBox();
}

void DrawArea::AddSelectByAttribute()
{
    auto fnSet = [](int nX, int nY)
    {
        if(g_editorMap.validC(nX, nY)){
            if(g_attributeSelectWindow->testLand(g_editorMap.cell(nX, nY).land)){
                g_editorMap.cellSelect(nX, nY).ground = g_mainWindow->Deselect() ? false : true;
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
                    &&  (g_editorMap.validC(nX, nY))){

                if(g_editorMap.tile(nX, nY).valid && g_editorMap.tileSelect(nX, nY).tile){
                    int nStartX = nX * SYS_MAPGRIDXP - offsetX;
                    int nStartY = nY * SYS_MAPGRIDYP - offsetY;
                    FillRectangle(nStartX, nStartY, SYS_MAPGRIDXP * 2, SYS_MAPGRIDYP * 2, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
                }
            }
        }
    }
}

void DrawArea::DrawDoneSelectByObject(int depth)
{
    const auto [offsetX, offsetY] = offset();

    int nX0 = offsetX / SYS_MAPGRIDXP - SYS_OBJMAXW;
    int nY0 = offsetY / SYS_MAPGRIDYP - SYS_OBJMAXH;

    int nX1 = (offsetX + w()) / SYS_MAPGRIDXP + SYS_OBJMAXW;
    int nY1 = (offsetY + h()) / SYS_MAPGRIDYP + SYS_OBJMAXH;

    for(int nX = nX0; nX < nX1; ++nX){
        for(int nY = nY0; nY < nY1; ++nY){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                if(g_editorMap.validC(nX, nY)){
                    const auto &obj = g_editorMap.cell(nX, nY).obj[nIndex];
                    if(true
                            && obj.valid
                            && obj.depth == depth
                            && g_editorMap.cellSelect(nX, nY).obj[nIndex]){
                        if(auto img = g_imageCache.retrieve(obj.texID)){
                            const int nW = img->w();
                            const int nH = img->h();
                            const int startX = nX * SYS_MAPGRIDXP - offsetX;
                            const int startY = nY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - nH;
                            FillRectangle(startX, startY, nW, nH, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
                        }
                    }
                }
            }
        }
    }
}

void DrawArea::DrawDoneSelectByObjectIndex(int objIndex)
{
    const auto [offsetX, offsetY] = offset();

    int nX0 = offsetX / SYS_MAPGRIDXP - SYS_OBJMAXW;
    int nY0 = offsetY / SYS_MAPGRIDYP - SYS_OBJMAXH;

    int nX1 = (offsetX + w()) / SYS_MAPGRIDXP + SYS_OBJMAXW;
    int nY1 = (offsetY + h()) / SYS_MAPGRIDYP + SYS_OBJMAXH;

    for(int nX = nX0; nX < nX1; ++nX){
        for(int nY = nY0; nY < nY1; ++nY){
            if(g_editorMap.validC(nX, nY)){
                const auto &obj = g_editorMap.cell(nX, nY).obj[objIndex];
                if(obj.valid && g_editorMap.cellSelect(nX, nY).obj[objIndex]){
                    if(auto img = g_imageCache.retrieve(obj.texID)){
                        const int nW = img->w();
                        const int nH = img->h();
                        const int startX = nX * SYS_MAPGRIDXP - offsetX;
                        const int startY = nY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - nH;
                        FillRectangle(startX, startY, nW, nH, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
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

    if(g_editorMap.validC(nX, nY) && g_editorMap.tile(nX, nY).valid){
        int nStartX = (nX / 2) * 2 * SYS_MAPGRIDXP - offsetX;
        int nStartY = (nY / 2) * 2 * SYS_MAPGRIDYP - offsetY;

        FillRectangle(nStartX, nStartY, SYS_MAPGRIDXP * 2,  SYS_MAPGRIDYP * 2, g_mainWindow->Deselect() ? 0X80FF0000 : 0X8000FF00);
        DrawFloatObject((nX / 2) * 2, (nY / 2) * 2, FOTYPE_TILE, m_mouseX - x(), m_mouseY - y());
    }
}

void DrawArea::DrawSelectByObject(int depth)
{
    const auto [offsetX, offsetY] = offset();

    int nMouseXOnMap = m_mouseX - x() + offsetX;
    int nMouseYOnMap = m_mouseY - y() + offsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    for(int nCurrY = nY - 1; nCurrY < g_editorMap.h(); ++nCurrY){
        if(g_editorMap.validC(nX, nCurrY)){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                const auto &obj = g_editorMap.cell(nX, nCurrY).obj[nIndex];
                if(obj.valid && obj.depth == depth){
                    if(auto img = g_imageCache.retrieve(obj.texID)){
                        const int nW = img->w();
                        const int nH = img->h();

                        const int nStartX = nX     * SYS_MAPGRIDXP - offsetX;
                        const int nStartY = nCurrY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - nH;

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

    if(g_editorMap.validC(nX, nY) && g_editorMap.tile(nX, nY).valid){
        g_editorMap.tileSelect(nX, nY).tile = true;
    }
}

void DrawArea::AddSelectByObject(int depth)
{
    const auto [offsetX, offsetY] = offset();

    int nMouseXOnMap = m_mouseX - x() + offsetX;
    int nMouseYOnMap = m_mouseY - y() + offsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    for(int nCurrY = nY - 1; nCurrY < g_editorMap.h(); ++nCurrY){
        if(g_editorMap.validC(nX, nCurrY)){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                const auto &obj = g_editorMap.cell(nX, nCurrY).obj[nIndex];
                if(obj.valid && obj.depth == depth){
                    if(auto pImage = g_imageCache.retrieve(obj.texID)){
                        int nW = pImage->w();
                        int nH = pImage->h();

                        int nStartX = nX     * SYS_MAPGRIDXP - offsetX;
                        int nStartY = nCurrY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - nH;

                        if(mathf::pointInRectangle(nMouseXOnMap - offsetX, nMouseYOnMap - offsetY, nStartX, nStartY, nW, nH)){
                            g_editorMap.cellSelect(nX, nCurrY).obj[nIndex] = !g_mainWindow->Deselect();
                            return;
                        }
                    }
                }
            }
        }
    }
}

void DrawArea::AddSelectByObjectIndex(int objIndex)
{
    fflassert(objIndex == 0 || objIndex == 1);
    const auto [offsetX, offsetY] = offset();

    int nMouseXOnMap = m_mouseX - x() + offsetX;
    int nMouseYOnMap = m_mouseY - y() + offsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    for(int nCurrY = nY - 1; nCurrY < g_editorMap.h(); ++nCurrY){
        if(g_editorMap.validC(nX, nCurrY)){
            const auto &obj = g_editorMap.cell(nX, nCurrY).obj[objIndex];
            if(obj.valid){
                if(auto pImage = g_imageCache.retrieve(obj.texID)){
                    int nW = pImage->w();
                    int nH = pImage->h();

                    int nStartX = nX     * SYS_MAPGRIDXP - offsetX;
                    int nStartY = nCurrY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - nH;

                    if(mathf::pointInRectangle(nMouseXOnMap - offsetX, nMouseYOnMap - offsetY, nStartX, nStartY, nW, nH)){
                        g_editorMap.cellSelect(nX, nCurrY).obj[objIndex] = !g_mainWindow->Deselect();
                        return;
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

    if(g_editorMap.validC(nX, nY)){
        g_editorMap.cellSelect(nX, nY).ground = (g_mainWindow->Deselect() ? false : true);
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
        if(g_editorMap.validC(nX, nY)){
            g_editorMap.cellSelect(nX, nY).ground = g_mainWindow->Deselect() ? false : true;
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
                if(g_editorMap.validC(nX + nMX, nY + nMY)){
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
        g_editorMap.cellSelect(nX, nY).ground = g_mainWindow->Deselect() ? false : true;
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

            if(g_attributeSelectWindow->testLand(g_editorMap.cell(nX, nY).land)){
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
            if(g_editorMap.validC(nX, nY)){
                if(g_editorMap.cellSelect(nX, nY).attribute == !g_mainWindow->Reversed()){
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
    DrawDoneSelectByObject(OBJD_GROUND);
    DrawDoneSelectByObject(OBJD_OVERGROUND0);
    DrawDoneSelectByObject(OBJD_OVERGROUND1);
    DrawDoneSelectByObject(OBJD_SKY);
}

void DrawArea::DrawTrySelect()
{
    if(g_mainWindow->EnableSelect()){
        auto nColor = fl_color();
        fl_color(FL_RED);

        if(g_mainWindow->selectByTile()){
            DrawTrySelectByTile();
        }

        for(const auto depth: {OBJD_GROUND, OBJD_OVERGROUND0, OBJD_OVERGROUND1, OBJD_SKY}){
            if(g_mainWindow->selectByObject(depth)){
                DrawDoneSelectByObject(depth);
            }
        }

        for(const auto objIndex: {0, 1}){
            if(g_mainWindow->selectByObjectIndex(objIndex)){
                DrawDoneSelectByObjectIndex(objIndex);
            }
        }

        if(g_mainWindow->selectBySingle()){
            DrawTrySelectBySingle();
        }

        if(g_mainWindow->selectByRhombus()){
            DrawTrySelectByRhombus();
        }

        if(g_mainWindow->selectByRectangle()){
            DrawTrySelectByRectangle();
        }

        if(g_mainWindow->selectByAttribute()){
            DrawTrySelectByAttribute();
        }

        fl_color(nColor);
    }
}

void DrawArea::DrawTextBox()
{
    FillRectangle(0, 0, 250, 150, 0XC0000000);
    const fl_wrapper::enable_color enable(255, 0, 0);

    int nY = 20;
    const auto [offsetX, offsetY] = offset();

    DrawText(10, nY, "OffsetX: %d %d", offsetX / SYS_MAPGRIDXP, offsetX); nY += 20;
    DrawText(10, nY, "OffsetY: %d %d", offsetY / SYS_MAPGRIDYP, offsetY); nY += 20;

    int nMouseX = (std::max<int>)(0, m_mouseX + offsetX - x());
    int nMouseY = (std::max<int>)(0, m_mouseY + offsetY - y());

    DrawText(10, nY, "MouseMX: %d %d", nMouseX / SYS_MAPGRIDXP, nMouseX); nY += 20;
    DrawText(10, nY, "MouseMY: %d %d", nMouseY / SYS_MAPGRIDYP, nMouseY); nY += 20;
}

void DrawArea::drawObject(int depth)
{
    if(!g_editorMap.valid()){
        return;
    }

    if(!g_mainWindow->ShowObject(depth)){
        return;
    }

    const fl_wrapper::enable_color enable([depth]()
    {
        switch(depth){
            case OBJD_GROUND     : return FL_RED;
            case OBJD_OVERGROUND0: return FL_BLUE;
            case OBJD_OVERGROUND1: return FL_MAGENTA;
            case OBJD_SKY        : return FL_GREEN;
            default              : throw bad_reach();
        }
    }());

    const auto [offsetX, offsetY] = offset();
    const int startGX = offsetX / SYS_MAPGRIDXP - 10;
    const int startGY = offsetY / SYS_MAPGRIDYP - 20;

    const int drawGW = w() / SYS_MAPGRIDXP + 20;
    const int drawGH = h() / SYS_MAPGRIDYP + 40;

    for(int iy = startGY; iy < startGY + drawGH; ++iy){
        for(int ix = startGX; ix < startGX + drawGW; ++ix){
            if(!g_editorMap.validC(ix, iy)){
                continue;
            }

            for(const int objIndex: {0, 1}){
                const auto &obj = g_editorMap.cell(ix, iy).obj[objIndex];
                if(obj.valid && obj.depth == depth){
                    const auto  fileIndex = to_u8 (obj.texID >> 16);
                    /* */ auto imageIndex = to_u16(obj.texID);

                    if(obj.animated){
                        // check here if fileIndex == 11, 26, 41, 56, 71, means it requires the obj comes from xxx/Animationsc.wil
                        // but can't confirm which mir2 version needs this, skip it
                        imageIndex += to_u16(m_aniTimer.frame(obj.tickType, obj.frameCount));
                    }

                    if(auto img = g_imageCache.retrieve((to_u32(fileIndex) << 16) + imageIndex)){
                        const int startX = ix * SYS_MAPGRIDXP - offsetX;
                        const int startY = iy * SYS_MAPGRIDYP + SYS_MAPGRIDYP - img->h() - offsetY;

                        drawImage(img, startX, startY);
                        if(g_mainWindow->ShowObjectLine(depth) || g_mainWindow->ShowObjectIndexLine(objIndex)){
                            drawRectangle(startX, startY, img->w(), img->h());
                        }
                    }
                }
            }
        }
    }
}

void DrawArea::DrawAttributeGrid()
{
    if(!g_mainWindow->ShowAttributeGridLine()){
        return;
    }

    const fl_wrapper::enable_color enable(FL_RED);
    const auto [offsetX, offsetY] = offset();

    int nX0 = offsetX / SYS_MAPGRIDXP - 1;
    int nY0 = offsetY / SYS_MAPGRIDYP - 1;
    int nX1 = (offsetX + w()) / SYS_MAPGRIDXP + 1;
    int nY1 = (offsetY + h()) / SYS_MAPGRIDYP + 1;

    for(int nCX = nX0; nCX < nX1; ++nCX){
        for(int nCY = nY0; nCY < nY1; ++nCY){
            if(g_editorMap.validC(nCX, nCY)){
                if(g_attributeSelectWindow->testLand(g_editorMap.cell(nCX, nCY).land)){
                    int nPX = nCX * SYS_MAPGRIDXP - offsetX;
                    int nPY = nCY * SYS_MAPGRIDYP - offsetY;
                    drawRectangle(nPX, nPY, SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                    drawLine(nPX, nPY, nPX + SYS_MAPGRIDXP, nPY + SYS_MAPGRIDYP);
                    drawLine(nPX + SYS_MAPGRIDXP, nPY, nPX, nPY + SYS_MAPGRIDYP);
                }
            }
        }
    }
}

void DrawArea::DrawGrid()
{
    if(!g_mainWindow->ShowGridLine()){
        return;
    }

    const auto [offsetX, offsetY] = offset();
    const fl_wrapper::enable_color enable(FL_MAGENTA);
    for(int nCX = offsetX / SYS_MAPGRIDXP - 1; nCX < (offsetX + w()) / SYS_MAPGRIDXP + 1; ++nCX){
        drawLine(nCX * SYS_MAPGRIDXP - offsetX, 0, nCX * SYS_MAPGRIDXP - offsetX, h());
    }

    for(int nCY = offsetY / SYS_MAPGRIDYP - 1; nCY < (offsetY + h()) / SYS_MAPGRIDYP + 1; ++nCY){
        drawLine(0, nCY * SYS_MAPGRIDYP - offsetY, w(), nCY * SYS_MAPGRIDYP - offsetY);
    }
}

void DrawArea::drawTile()
{
    if(!g_editorMap.valid()){
        return;
    }

    if(!g_mainWindow->ShowTile()){
        return;
    }

    const auto [offsetX, offsetY] = offset();
    const fl_wrapper::enable_color enable(FL_RED);

    const int startGX = offsetX / SYS_MAPGRIDXP - 1;
    const int startGY = offsetY / SYS_MAPGRIDYP - 1;

    const int drawGW = w() / SYS_MAPGRIDXP + 3;
    const int drawGH = h() / SYS_MAPGRIDYP + 8;

    for(int iy = startGY; iy < startGY + drawGH; ++iy){
        for(int ix = startGX; ix < startGX + drawGW; ++ix){
            if(!g_editorMap.validC(ix, iy)){
                continue;
            }

            if(ix % 2 || iy % 2){
                continue;
            }

            if(!g_editorMap.tile(ix, iy).valid){
                continue;
            }

            const int startX = ix * SYS_MAPGRIDXP - offsetX;
            const int startY = iy * SYS_MAPGRIDYP - offsetY;

            if(auto img = g_imageCache.retrieve(g_editorMap.tile(ix, iy).texID)){
                drawImage(img, startX, startY);
                if(g_mainWindow->ShowTileLine()){
                    drawRectangle(startX, startY, img->w(), img->h());
                }
            }
        }
    }
}

int DrawArea::handle(int event)
{
    auto result = BaseArea::handle(event);

    // can't find resize event
    // put it here as a hack and check it every time
    g_mainWindow->checkScrollBar();

    if(g_editorMap.valid()){
        const int lastMouseX = m_mouseX;
        const int lastMouseY = m_mouseY;

        m_mouseX = Fl::event_x();
        m_mouseY = Fl::event_y();

        switch(event){
            case FL_FOCUS:
            case FL_UNFOCUS:
                {
                    result = 1;
                    break;
                }
            case FL_KEYDOWN:
                {
                    const auto [dx, dy] = [this]() -> std::tuple<int, int>
                    {
                        switch(Fl::event_key()){
                            case FL_Up   : return { 0, -1};
                            case FL_Down : return { 0,  1};
                            case FL_Left : return {-1,  0};
                            case FL_Right: return { 1,  0};
                            default      : return { 0,  0};
                        }
                    }();

                    const auto [dxratio, dyratio] = getScrollPixelRatio(SYS_MAPGRIDXP * dx, SYS_MAPGRIDYP * dy);
                    g_mainWindow->addScrollBarValue(dxratio, dyratio);
                    break;
                }
            case FL_MOUSEWHEEL:
                {
                    const auto [dxratio, dyratio] = getScrollPixelRatio(Fl::event_dx() * SYS_MAPGRIDXP, Fl::event_dy() * SYS_MAPGRIDYP);
                    g_mainWindow->addScrollBarValue(dxratio, dyratio);
                    result = 1;
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
                            const auto [xratio, yratio] = getScrollPixelRatio(lastMouseX - m_mouseX, lastMouseY - m_mouseY);
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

                    // for drag event
                    result = 1;
                    break;
                }
            default:
                {
                    break;
                }
        }
    }

    g_mainWindow->redrawAll();
    return result;
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

void DrawArea::clearGroundSelect()
{
    // if(g_editorMap.valid()){
    //     g_editorMap.clearGroundSelect();
    // }
}

void DrawArea::AddSelect()
{
    if(g_mainWindow->selectBySingle()){
        AddSelectBySingle();
    }

    if(g_mainWindow->selectByRhombus()){
        AddSelectByRhombus();
    }

    if(g_mainWindow->selectByRectangle()){
        AddSelectByRectangle();
    }

    if(g_mainWindow->selectByAttribute()){
        AddSelectByAttribute();
    }
    
    if(g_mainWindow->selectByTile()){
        AddSelectByTile();
    }

    if(g_mainWindow->selectByObject(true)){
        AddSelectByObject(true);
    }

    if(g_mainWindow->selectByObject(false)){
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
                if(g_editorMap.validC(nX, nY)){
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
            drawImage(m_lightImge.get(),
                    nX * SYS_MAPGRIDXP - offsetX + SYS_MAPGRIDXP / 2 - (m_lightImge->w() - 1) / 2,
                    nY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP / 2 - (m_lightImge->h() - 1) / 2);

            if(g_mainWindow->ShowLightLine()){
                const fl_wrapper::enable_color enable(FL_RED);
                DrawCircle(nX * SYS_MAPGRIDXP - offsetX, nY * SYS_MAPGRIDYP - offsetY, 10);
            }
        };

        int nX = offsetX / SYS_MAPGRIDXP - 1;
        int nY = offsetY / SYS_MAPGRIDYP - 1;
        int nW = w() / SYS_MAPGRIDXP + 3;
        int nH = h() / SYS_MAPGRIDYP + 8;

        g_editorMap.drawLight(nX, nY, nW, nH, fnDrawLight);
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
            && g_editorMap.validC(nX, nY)){

        uint8_t  nFileIndex  = 0;
        uint16_t nImageIndex = 0;

        Fl_Image *pImage = nullptr;
        switch(nFOType){
            case FOTYPE_TILE:
                {
                    if(true
                            && !(nX % 2)
                            && !(nY % 2)){

                        if(g_editorMap.tile(nX, nY).valid){
                            pImage = g_imageCache.retrieve(g_editorMap.tile(nX, nY).texID);
                        }
                    }
                    break;
                }
            case FOTYPE_OBJ0:
            case FOTYPE_OBJ1:
                {
                    const auto &obj = g_editorMap.cell(nX, nY).obj[(nFOType == FOTYPE_OBJ0) ? 0 : 1];
                    if(obj.valid){
                        pImage = g_imageCache.retrieve(obj.texID);
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
            drawImage(pImage, nImageX, nImageY);

            // draw boundary for window
            {
                const fl_wrapper::enable_color enable(FL_YELLOW);
                drawRectangle(nWinX, nWinY, nWinW, nWinH);
            }

            // draw boundary for image
            {
                const fl_wrapper::enable_color enable(FL_MAGENTA);
                drawRectangle(nImageX, nImageY, pImage->w(), pImage->h());
            }

            // draw textbox
            // after we allocated textbox
            // we have small offset to start inside it
            {
                const fl_wrapper::enable_color enable(FL_BLUE);
                drawRectangle(nTextBoxX, nTextBoxY, nTextBoxW, nTextBoxH);
            }


            int nTextOffX = 20;
            int nTextOffY = 20;

            switch(nFOType){
                case FOTYPE_TILE:
                    {
                        const fl_wrapper::enable_color enable(FL_RED);

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
                        break;
                    }
                case FOTYPE_OBJ0:
                case FOTYPE_OBJ1:
                    {
                        const fl_wrapper::enable_color enable(FL_RED);

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

                        const auto &obj = g_editorMap.cell(nX, nY).obj[(nFOType == FOTYPE_OBJ0) ? 0 : 1];
                        DrawText(nTextStartX, nTextStartY, "ABlend : %s", obj.alpha ? "yes" : "no");
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "Animat : %s", obj.animated ? "yes" : "no");
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "AniTyp : %d", obj.tickType);
                        nTextStartY += 20;

                        DrawText(nTextStartX, nTextStartY, "AniCnt : %d", obj.frameCount);
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

std::optional<std::tuple<size_t, size_t>> DrawArea::getROISize() const
{
    if(g_editorMap.valid()){
        return std::make_tuple(g_editorMap.w() * SYS_MAPGRIDXP, g_editorMap.h() * SYS_MAPGRIDYP);
    }
    return {};
}

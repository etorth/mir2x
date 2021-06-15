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

    drawAttributeGrid();

    drawLight();
    drawGrid();

    drawDoneSelect();
    drawTrySelect();
    drawTextBox();
}

void DrawArea::addSelectByAttribute()
{
    const auto [mouseGX, mouseGY] = mouseGrid();
    if(g_editorMap.validC(mouseGX, mouseGY)){
        g_editorMap.cellSelect(mouseGX, mouseGY).attribute = (g_mainWindow->deselect() ? false : true);
    }
}

void DrawArea::drawDoneSelectByTile()
{
    const auto [offsetX, offsetY] = offset();

    const int startGX = offsetX / SYS_MAPGRIDXP - 1;
    const int startGY = offsetY / SYS_MAPGRIDYP - 1;

    const int drawGW = w() / SYS_MAPGRIDXP + 10;
    const int drawGH = h() / SYS_MAPGRIDYP + 40;

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
            fillRectangle(startX, startY, SYS_MAPGRIDXP * 2, SYS_MAPGRIDYP * 2, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
        }
    }
}

void DrawArea::drawDoneSelectByObject(int depth)
{
    const auto [offsetX, offsetY] = offset();

    const int startGX = offsetX / SYS_MAPGRIDXP - 1;
    const int startGY = offsetY / SYS_MAPGRIDYP - 1;

    const int drawGW = w() / SYS_MAPGRIDXP + 10;
    const int drawGH = h() / SYS_MAPGRIDYP + 40;

    for(int iy = startGY; iy < startGY + drawGH; ++iy){
        for(int ix = startGX; ix < startGX + drawGW; ++ix){
            if(!g_editorMap.validC(ix, iy)){
                continue;
            }

            for(const auto objIndex: {0, 1}){
                const auto &obj = g_editorMap.cell(ix, iy).obj[objIndex];
                if(true
                        && obj.valid
                        && obj.depth == depth
                        && g_editorMap.cellSelect(ix, iy).obj[objIndex]){
                    if(auto img = g_imageCache.retrieve(obj.texID)){
                        const int startX = ix * SYS_MAPGRIDXP - offsetX;
                        const int startY = iy * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - img->h();
                        fillRectangle(startX, startY, img->w(), img->h(), g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
                    }
                }
            }
        }
    }
}

void DrawArea::drawDoneSelectByObjectIndex(int objIndex)
{
    const auto [offsetX, offsetY] = offset();

    const int startGX = offsetX / SYS_MAPGRIDXP - 1;
    const int startGY = offsetY / SYS_MAPGRIDYP - 1;

    const int drawGW = w() / SYS_MAPGRIDXP + 10;
    const int drawGH = h() / SYS_MAPGRIDYP + 40;

    for(int iy = startGY; iy < startGY + drawGH; ++iy){
        for(int ix = startGX; ix < startGX + drawGW; ++ix){
            if(!g_editorMap.validC(ix, iy)){
                continue;
            }

            const auto &obj = g_editorMap.cell(ix, iy).obj[objIndex];
            if(obj.valid && g_editorMap.cellSelect(ix, iy).obj[objIndex]){
                if(auto img = g_imageCache.retrieve(obj.texID)){
                    const int startX = ix * SYS_MAPGRIDXP - offsetX;
                    const int startY = iy * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - img->h();
                    fillRectangle(startX, startY, img->w(), img->h(), g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
                }
            }
        }
    }
}

void DrawArea::drawTrySelectByTile()
{
    const auto [mouseGX, mouseGY] = mouseGrid();
    if(g_editorMap.validC(mouseGX, mouseGY) && g_editorMap.tile(mouseGX, mouseGY).valid){
        const auto tileGX = (mouseGX / 2) * 2;
        const auto tileGY = (mouseGY / 2) * 2;
        fillGrid(tileGX, tileGY, 2, 2, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
        drawFloatObject(tileGX, tileGY, FOTYPE_TILE, m_mouseX - x(), m_mouseY - y());
    }
}

void DrawArea::drawSelectByObject(int depth)
{
    const auto [offsetX, offsetY] = offset();

    int nMouseXOnMap = m_mouseX - x() + offsetX;
    int nMouseYOnMap = m_mouseY - y() + offsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    for(int nCurrY = nY - 1; nCurrY < to_d(g_editorMap.h()); ++nCurrY){
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
                            fillRectangle(nStartX, nStartY, nW, nH, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
                            drawFloatObject(nX, nCurrY, (nIndex == 0) ? FOTYPE_OBJ0 : FOTYPE_OBJ1, m_mouseX - x(), m_mouseY - y());
                            return;
                        }
                    }
                }
            }
        }
    }
}

void DrawArea::drawTrySelectBySingle()
{
    const auto [mouseGX, mouseGY] = mouseGrid();
    fillGrid(mouseGX, mouseGY, 1, 1, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
}

void DrawArea::addSelectByTile()
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

void DrawArea::addSelectByObject(int depth)
{
    const auto [offsetX, offsetY] = offset();

    int nMouseXOnMap = m_mouseX - x() + offsetX;
    int nMouseYOnMap = m_mouseY - y() + offsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    for(int nCurrY = nY - 1; nCurrY < to_d(g_editorMap.h()); ++nCurrY){
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
                            g_editorMap.cellSelect(nX, nCurrY).obj[nIndex] = !g_mainWindow->deselect();
                            return;
                        }
                    }
                }
            }
        }
    }
}

void DrawArea::addSelectByObjectIndex(int objIndex)
{
    fflassert(objIndex == 0 || objIndex == 1);
    const auto [offsetX, offsetY] = offset();

    int nMouseXOnMap = m_mouseX - x() + offsetX;
    int nMouseYOnMap = m_mouseY - y() + offsetY;

    int nX = nMouseXOnMap / SYS_MAPGRIDXP;
    int nY = nMouseYOnMap / SYS_MAPGRIDYP;

    for(int nCurrY = nY - 1; nCurrY < to_d(g_editorMap.h()); ++nCurrY){
        if(g_editorMap.validC(nX, nCurrY)){
            const auto &obj = g_editorMap.cell(nX, nCurrY).obj[objIndex];
            if(obj.valid){
                if(auto pImage = g_imageCache.retrieve(obj.texID)){
                    int nW = pImage->w();
                    int nH = pImage->h();

                    int nStartX = nX     * SYS_MAPGRIDXP - offsetX;
                    int nStartY = nCurrY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - nH;

                    if(mathf::pointInRectangle(nMouseXOnMap - offsetX, nMouseYOnMap - offsetY, nStartX, nStartY, nW, nH)){
                        g_editorMap.cellSelect(nX, nCurrY).obj[objIndex] = !g_mainWindow->deselect();
                        return;
                    }
                }
            }
        }
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

void DrawArea::drawTrySelectByRhombus()
{
    auto fnDraw = [this](int nX, int nY)
    {
        fillGrid(nX, nY, 1, 1, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
    };

    const auto [offsetX, offsetY] = offset();

    int nMX = m_mouseX + offsetX - x();
    int nMY = m_mouseY + offsetY - y();

    RhombusCoverOperation(nMX, nMY, g_selectSettingWindow->RhombusSize(), fnDraw);
}

void DrawArea::addSelectByRhombus()
{
    auto fnSet = [](int nX, int nY)
    {
        if(g_editorMap.validC(nX, nY)){
            g_editorMap.cellSelect(nX, nY).ground = g_mainWindow->deselect() ? false : true;
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

void DrawArea::drawTrySelectByRectangle()
{
    auto fnDraw = [this](int nX, int nY)
    {
        fillGrid(nX, nY, 1, 1, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
    };

    const auto [offsetX, offsetY] = offset();

    int nMX = m_mouseX + offsetX - x();
    int nMY = m_mouseY + offsetY - y();

    RectangleCoverOperation(nMX, nMY, g_selectSettingWindow->RectangleSize(), fnDraw);
}

void DrawArea::addSelectByRectangle()
{
    auto fnSet = [](int nX,  int nY)
    {
        g_editorMap.cellSelect(nX, nY).ground = g_mainWindow->deselect() ? false : true;
    };

    const auto [offsetX, offsetY] = offset();

    int nMX = m_mouseX + offsetX - x();
    int nMY = m_mouseY + offsetY - y();

    RectangleCoverOperation(nMX, nMY, g_selectSettingWindow->RectangleSize(), fnSet);
}

void DrawArea::drawTrySelectByAttribute()
{
    int nSize = g_selectSettingWindow->AttributeSize();

    if(nSize > 0){

        const auto [offsetX, offsetY] = offset();

        int nMX = m_mouseX + offsetX - x();
        int nMY = m_mouseY + offsetY - y();

        auto fnDraw = [this](int nX, int nY) -> void
        {

            if(g_attributeSelectWindow->testLand(g_editorMap.cell(nX, nY).land)){
                fillGrid(nX, nY, 1, 1, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
            }
        };

        AttributeCoverOperation(nMX, nMY, nSize, fnDraw);
    }
}

void DrawArea::drawDoneSelectByAttribute()
{
    const auto [offsetX, offsetY] = offset();

    const int startGX = offsetX / SYS_MAPGRIDXP - 1;
    const int startGY = offsetY / SYS_MAPGRIDYP - 1;

    const int drawGW = w() / SYS_MAPGRIDXP + 10;
    const int drawGH = h() / SYS_MAPGRIDYP + 40;

    for(int iy = startGY; iy < startGY + drawGH; ++iy){
        for(int ix = startGX; ix < startGX + drawGW; ++ix){
            if(g_editorMap.validC(ix, iy)){
                if(g_editorMap.cellSelect(ix, iy).attribute == !g_mainWindow->reversed()){
                    fillGrid(ix, iy, 1, 1, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
                }
            }
        }
    }
}

void DrawArea::drawDoneSelect()
{
    drawDoneSelectByTile();
    drawDoneSelectByAttribute();
    drawDoneSelectByObject(OBJD_GROUND);
    drawDoneSelectByObject(OBJD_OVERGROUND0);
    drawDoneSelectByObject(OBJD_OVERGROUND1);
    drawDoneSelectByObject(OBJD_SKY);
}

void DrawArea::drawTrySelect()
{
    if(!g_mainWindow->enableSelect()){
        return;
    }

    if(g_mainWindow->enableSelectByTile()){
        drawTrySelectByTile();
    }

    for(const auto depth: {OBJD_GROUND, OBJD_OVERGROUND0, OBJD_OVERGROUND1, OBJD_SKY}){
        if(g_mainWindow->enableSelectByObject(depth)){
            drawDoneSelectByObject(depth);
        }
    }

    for(const auto objIndex: {0, 1}){
        if(g_mainWindow->enableSelectByObjectIndex(objIndex)){
            drawDoneSelectByObjectIndex(objIndex);
        }
    }

    if(g_mainWindow->enableSelectByAttribute()){
        drawTrySelectByAttribute();
    }

    drawTrySelectBySingle();
}

void DrawArea::drawTextBox()
{
    fillRectangle(0, 0, 250, 150, 0XC0000000);
    const fl_wrapper::enable_color enable(FL_RED);

    int startY = 20;
    const auto [offsetX, offsetY] = offset();

    drawText(10, startY, "OffsetX: %d %d", offsetX / SYS_MAPGRIDXP, offsetX); startY += 20;
    drawText(10, startY, "OffsetY: %d %d", offsetY / SYS_MAPGRIDYP, offsetY); startY += 20;

    const int mousePX = std::max<int>(0, m_mouseX + offsetX - x());
    const int mousePY = std::max<int>(0, m_mouseY + offsetY - y());

    drawText(10, startY, "MouseGX: %d %d", mousePX / SYS_MAPGRIDXP, mousePX); startY += 20;
    drawText(10, startY, "MouseGY: %d %d", mousePY / SYS_MAPGRIDYP, mousePY); startY += 20;
}

void DrawArea::drawObject(int depth)
{
    if(!g_editorMap.valid()){
        return;
    }

    const auto [offsetX, offsetY] = offset();
    const int startGX = offsetX / SYS_MAPGRIDXP - 1;
    const int startGY = offsetY / SYS_MAPGRIDYP - 1;

    const int drawGW = w() / SYS_MAPGRIDXP + 10;
    const int drawGH = h() / SYS_MAPGRIDYP + 40;

    const fl_wrapper::enable_color enable(fl_wrapper::color(depth));
    for(int iy = startGY; iy < startGY + drawGH; ++iy){
        for(int ix = startGX; ix < startGX + drawGW; ++ix){
            if(!g_editorMap.validC(ix, iy)){
                continue;
            }

            for(const int objIndex: {0, 1}){
                const auto &obj = g_editorMap.cell(ix, iy).obj[objIndex];
                if(obj.valid && obj.depth == depth){
                    if(g_mainWindow->enableShowObject(depth) || g_mainWindow->enableShowObjectIndex(objIndex)){
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
                            if(g_mainWindow->enableShowObjectLine(depth) || g_mainWindow->enableShowObjectIndexLine(objIndex)){
                                drawRectangle(startX, startY, img->w(), img->h());
                            }
                        }
                    }
                }
            }
        }
    }
}

void DrawArea::drawAttributeGrid()
{
    if(!g_mainWindow->enableShowAttributeGridLine()){
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

void DrawArea::drawGrid()
{
    if(!g_mainWindow->enableShowGridLine()){
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

    if(!g_mainWindow->enableShowTile()){
        return;
    }

    const auto [offsetX, offsetY] = offset();
    const fl_wrapper::enable_color enable(FL_RED);

    const int startGX = offsetX / SYS_MAPGRIDXP - 1;
    const int startGY = offsetY / SYS_MAPGRIDYP - 1;

    const int drawGW = w() / SYS_MAPGRIDXP + 10;
    const int drawGH = h() / SYS_MAPGRIDYP + 40;

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
                if(g_mainWindow->enableShowTileLine()){
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
                    if(g_mainWindow->enableSelect()){
                        addSelect();
                    }
                    else if(g_mainWindow->EnableEdit()){
                        // TODO
                        //
                    }
                    else if(g_mainWindow->enableTest()){
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
                    if(g_mainWindow->enableSelect()){
                        addSelect();
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
    }
    else{
        fl_alert("Invalid radius for CreateRoundImage(%d, 0X%08X)", nRadius, nColor);
        return nullptr;
    }
}

void DrawArea::clearSelect()
{
    if(g_editorMap.valid()){
        g_editorMap.clearSelect();
    }
}

void DrawArea::addSelect()
{
    const auto [mouseGX, mouseGY] = mouseGrid();
    if(!g_editorMap.validC(mouseGX, mouseGY)){
        return;
    }

    if(g_mainWindow->enableSelectByAttribute()){
        g_editorMap.cellSelect(mouseGX, mouseGY).attribute = !g_mainWindow->deselect();
    }
    else if(g_mainWindow->enableSelectByTile()){
        g_editorMap.tileSelect(mouseGX, mouseGY).tile = !g_mainWindow->deselect();
    }
    else{
        for(const auto objIndex: {0, 1}){
            const auto &obj = g_editorMap.cell(mouseGX, mouseGY).obj[objIndex];
            if(false
                    || g_mainWindow->enableSelectByObject(obj.depth)
                    || g_mainWindow->enableSelectByObjectIndex(objIndex)){
                g_editorMap.cellSelect(mouseGX, mouseGY).obj[objIndex] = !g_mainWindow->deselect();
            }
        }
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

void DrawArea::drawLight()
{
    if(!g_editorMap.valid()){
        return;
    }

    if(!g_mainWindow->enableShowLight()){
        return;
    }

    const auto [offsetX, offsetY] = offset();

    const int startGX = offsetX / SYS_MAPGRIDXP - 1;
    const int startGY = offsetY / SYS_MAPGRIDYP - 1;

    const int drawGW = w() / SYS_MAPGRIDXP + 10;
    const int drawGH = h() / SYS_MAPGRIDYP + 40;

    const fl_wrapper::enable_color enable(FL_MAGENTA);
    for(int iy = startGY; iy < startGY + drawGH; ++iy){
        for(int ix = startGX; ix < startGX + drawGW; ++ix){
            if(!g_editorMap.validC(ix, iy)){
                continue;
            }

            if(g_editorMap.cell(ix, iy).light.valid){
                const int drawCX = ix * SYS_MAPGRIDXP - offsetX + SYS_MAPGRIDXP / 2 - (m_lightImge->w() - 1) / 2;
                const int drawCY = iy * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP / 2 - (m_lightImge->h() - 1) / 2;
                drawImage(m_lightImge.get(), drawCX, drawCY);

                if(g_mainWindow->enableShowLightLine()){
                    drawCircle(ix * SYS_MAPGRIDXP - offsetX, iy * SYS_MAPGRIDYP - offsetY, 20);
                }
            }
        }
    }
}

void DrawArea::drawFloatObject(int nX, int nY, int nFOType, int nWinX, int nWinY)
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

            fillRectangle(nWinX, nWinY, nWinW, nWinH, 0XC0000000);
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

                        drawText(nTextStartX, nTextStartY, "     Tile");
                        nTextStartY += 20;

                        drawText(nTextStartX, nTextStartY, "Index0 : %d", to_d(nFileIndex));
                        nTextStartY += 20;

                        drawText(nTextStartX, nTextStartY, "Index1 : %d", to_d(nImageIndex));
                        nTextStartY += 20;

                        drawText(nTextStartX, nTextStartY, "DBName : %s", g_imageDB->dbName(nFileIndex));
                        nTextStartY += 20;
                        break;
                    }
                case FOTYPE_OBJ0:
                case FOTYPE_OBJ1:
                    {
                        const fl_wrapper::enable_color enable(FL_RED);

                        int nTextStartX = nTextBoxX + nTextOffX;
                        int nTextStartY = nTextBoxY + nTextOffY;

                        drawText(nTextStartX, nTextStartY, "    Obj[%d]", (nFOType == FOTYPE_OBJ0) ? 0 : 1);
                        nTextStartY += 20;

                        drawText(nTextStartX, nTextStartY, "Index0 : %d", to_d(nFileIndex));
                        nTextStartY += 20;

                        drawText(nTextStartX, nTextStartY, "Index1 : %d", to_d(nImageIndex));
                        nTextStartY += 20;

                        drawText(nTextStartX, nTextStartY, "DBName : %s", g_imageDB->dbName(nFileIndex));
                        nTextStartY += 20;

                        const auto &obj = g_editorMap.cell(nX, nY).obj[(nFOType == FOTYPE_OBJ0) ? 0 : 1];
                        drawText(nTextStartX, nTextStartY, "ABlend : %s", obj.alpha ? "yes" : "no");
                        nTextStartY += 20;

                        drawText(nTextStartX, nTextStartY, "Animat : %s", obj.animated ? "yes" : "no");
                        nTextStartY += 20;

                        drawText(nTextStartX, nTextStartY, "AniTyp : %d", obj.tickType);
                        nTextStartY += 20;

                        drawText(nTextStartX, nTextStartY, "AniCnt : %d", obj.frameCount);
                        nTextStartY += 20;

                        const auto imgInfo = g_imageDB->setIndex(nFileIndex, nImageIndex);
                        drawText(nTextStartX, nTextStartY, "OffseX : %d", to_d(imgInfo->px));
                        nTextStartY += 20;

                        drawText(nTextStartX, nTextStartY, "OffseY : %d", to_d(imgInfo->py));
                        nTextStartY += 20;

                        const auto &rstHeader = g_imageDB->getPackage(nFileIndex)->header();
                        auto nVersion = rstHeader.version;

                        drawText(nTextStartX, nTextStartY, "Versio : %d", to_d(nVersion));
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

std::optional<std::tuple<size_t, size_t>> DrawArea::getROISize() const
{
    if(g_editorMap.valid()){
        return std::make_tuple(g_editorMap.w() * SYS_MAPGRIDXP, g_editorMap.h() * SYS_MAPGRIDYP);
    }
    return {};
}

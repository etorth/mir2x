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

            if(!g_editorMap.tileSelect(ix, iy).tile){
                continue;
            }

            fillGrid(ix, iy, 2, 2, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
        }
    }
}

void DrawArea::drawDoneSelectByObject()
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

void DrawArea::drawTrySelectByTile()
{
    const auto [mouseGX, mouseGY] = mouseGrid();
    if(g_editorMap.validC(mouseGX, mouseGY) && g_editorMap.tile(mouseGX, mouseGY).valid){
        const auto tileGX = (mouseGX / 2) * 2;
        const auto tileGY = (mouseGY / 2) * 2;
        fillGrid(tileGX, tileGY, 2, 2, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
        drawFloatObject(tileGX, tileGY, FOBJ_TILE, m_mouseX - x(), m_mouseY - y());
    }
}

void DrawArea::drawTrySelectByObject()
{
    const auto [offsetX, offsetY] = offset();
    const auto [mouseGX, mouseGY] = mouseGrid();

    for(int currGY = mouseGY; currGY < mouseGY + 26; ++currGY){
        if(!g_editorMap.validC(mouseGX, currGY)){
            continue;
        }

        for(const auto objIndex: {0, 1}){
            const auto &obj = g_editorMap.cell(mouseGX, currGY).obj[objIndex];
            if(!obj.valid){
                continue;
            }

            if(false
                    || g_mainWindow->enableSelectByObject(obj.depth)
                    || g_mainWindow->enableSelectByObjectIndex(objIndex)){

                if(auto img = g_imageCache.retrieve(obj.texID)){
                    const int imgW = img->w();
                    const int imgH = img->h();

                    const int startX = mouseGX * SYS_MAPGRIDXP - offsetX;
                    const int startY =  currGY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - imgH;

                    if(mathf::pointInRectangle(m_mouseX - x(), m_mouseY - y(), startX, startY, imgW, imgH)){
                        fillRectangle(startX, startY, imgW, imgH, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
                        drawFloatObject(mouseGX, currGY, (objIndex == 0) ? FOBJ_OBJ0 : FOBJ_OBJ1, m_mouseX - x(), m_mouseY - y());
                        break;
                    }
                }
            }
        }
    }
}

void DrawArea::drawTrySelectByAttribute()
{
    const auto [mouseGX, mouseGY] = mouseGrid();
    if(!g_editorMap.validC(mouseGX, mouseGY)){
        return;
    }

    if(g_attributeSelectWindow->testLand(g_editorMap.cell(mouseGX, mouseGY).land)){
        fillGrid(mouseGX, mouseGY, 1, 1, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
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
            if(!g_editorMap.validC(ix, iy)){
                continue;
            }

            if(g_editorMap.cellSelect(ix, iy).attribute == g_mainWindow->reversed()){
                continue;
            }

            fillGrid(ix, iy, 1, 1, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
        }
    }
}

void DrawArea::drawDoneSelect()
{
    // paint all selected tiles/attributes/objects
    // selected doesn't need to check g_mainWindow->enableSelectByXXX

    drawDoneSelectByTile();
    drawDoneSelectByAttribute();
    drawDoneSelectByObject();
}

void DrawArea::drawTrySelect()
{
    if(!g_mainWindow->enableSelect()){
        return;
    }

    if(g_mainWindow->enableSelectByTile()){
        drawTrySelectByTile();
    }

    if(false
            || g_mainWindow->enableSelectByObject(OBJD_GROUND)
            || g_mainWindow->enableSelectByObject(OBJD_OVERGROUND0)
            || g_mainWindow->enableSelectByObject(OBJD_OVERGROUND1)
            || g_mainWindow->enableSelectByObject(OBJD_SKY)

            || g_mainWindow->enableSelectByObjectIndex(0)
            || g_mainWindow->enableSelectByObjectIndex(1)){
        drawTrySelectByObject();
    }

    if(g_mainWindow->enableSelectByAttribute()){
        drawTrySelectByAttribute();
    }

    const auto [mouseGX, mouseGY] = mouseGrid();
    fillGrid(mouseGX, mouseGY, 1, 1, g_mainWindow->deselect() ? 0X80FF0000 : 0X8000FF00);
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
        if(g_attributeSelectWindow->testLand(g_editorMap.cell(mouseGX, mouseGY).land)){
            g_editorMap.cellSelect(mouseGX, mouseGY).attribute = !g_mainWindow->deselect();
        }
    }

    if(g_mainWindow->enableSelectByTile()){
        g_editorMap.tileSelect(mouseGX, mouseGY).tile = !g_mainWindow->deselect();
    }

    else{
        const auto [offsetX, offsetY] = offset();
        for(int currGY = mouseGY; currGY < mouseGY + 26; ++currGY){
            if(!g_editorMap.validC(mouseGX, currGY)){
                continue;
            }

            for(const auto objIndex: {0, 1}){
                const auto &obj = g_editorMap.cell(mouseGX, currGY).obj[objIndex];
                if(!obj.valid){
                    continue;
                }

                if(false
                        || g_mainWindow->enableSelectByObject(obj.depth)
                        || g_mainWindow->enableSelectByObjectIndex(objIndex)){

                    if(auto img = g_imageCache.retrieve(obj.texID)){
                        const int imgW = img->w();
                        const int imgH = img->h();

                        const int startX = mouseGX * SYS_MAPGRIDXP - offsetX;
                        const int startY =  currGY * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP - imgH;

                        if(mathf::pointInRectangle(m_mouseX - x(), m_mouseY - y(), startX, startY, imgW, imgH)){
                            g_editorMap.cellSelect(mouseGX, currGY).obj[objIndex] = !g_mainWindow->deselect();
                            break;
                        }
                    }
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
                    drawCircle(ix * SYS_MAPGRIDXP - offsetX + SYS_MAPGRIDXP / 2, iy * SYS_MAPGRIDYP - offsetY + SYS_MAPGRIDYP / 2, 20);
                }
            }
        }
    }
}

void DrawArea::drawFloatObject(int gridX, int gridY, int floatObj, int floatWinX, int floatWinY)
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

    if(!g_editorMap.validC(gridX, gridY)){
        return;
    }

    if(!(floatObj == FOBJ_TILE || floatObj == FOBJ_OBJ0 || floatObj == FOBJ_OBJ1)){
        return;
    }

    const auto imageIndex = [floatObj, gridX, gridY]() -> uint32_t
    {
        switch(floatObj){
            case FOBJ_TILE:
                {
                    if(g_editorMap.tile(gridX, gridY).valid){
                        return g_editorMap.tile(gridX, gridY).texID;
                    }
                    return SYS_TEXNIL;
                }
            case FOBJ_OBJ0:
            case FOBJ_OBJ1:
                {
                    if(const auto &obj = g_editorMap.cell(gridX, gridY).obj[(floatObj == FOBJ_OBJ0) ? 0 : 1]; obj.valid){
                        return obj.texID;
                    }
                    return SYS_TEXNIL;
                }
            default:
                {
                    return SYS_TEXNIL;
                }
        }
    }();

    if(imageIndex == SYS_TEXNIL){
        return;
    }

    auto objImage = g_imageCache.retrieve(imageIndex);
    if(!objImage){
        return;
    }

    // setup text box size
    // based on different image type

    const int  objFileIndex = to_d(imageIndex >> 16);
    const int objImageIndex = to_d(imageIndex && 0X0000FFFF);

    const auto [textBoxW, textBoxH] = [objFileIndex, floatObj]() -> std::tuple<int, int>
    {
        switch(floatObj){
            case FOBJ_TILE:
                {
                    return
                    {
                        100 + std::strlen(g_imageDB->dbName(objFileIndex)) * 10,
                        150,
                    };
                }
            case FOBJ_OBJ0:
            case FOBJ_OBJ1:
            default:
                {
                    return
                    {
                        100 + std::strlen(g_imageDB->dbName(objFileIndex)) * 10,
                        260,
                    };
                }
        }
    }();

    const int marginTop    = 30;
    const int marginBottom = 30;

    const int marginLeft   = 30;
    const int marginMiddle = 30;
    const int marginRight  = 30;

    const int floatWinW = marginLeft + marginMiddle + marginRight + objImage->w() + textBoxW;
    const int floatWinH = marginTop  + marginBottom + std::max<int>(objImage->h(), textBoxH);

    const int imageStartX = floatWinX + marginLeft;
    const int imageStartY = floatWinY + (floatWinH - objImage->h()) / 2;

    const int textBoxX = floatWinX + marginLeft + objImage->w() + marginMiddle;
    const int textBoxY = floatWinY + (floatWinH - textBoxH) / 2;

    fillRectangle(floatWinX, floatWinY, floatWinW, floatWinH, 0XC0000000);
    {
        const fl_wrapper::enable_color enable(FL_YELLOW);
        drawRectangle(floatWinX, floatWinY, floatWinW, floatWinH);
    }

    drawImage(objImage, imageStartX, imageStartY);
    {
        const fl_wrapper::enable_color enable(FL_MAGENTA);
        drawRectangle(imageStartX, imageStartY, objImage->w(), objImage->h());
    }

    // draw textbox frame
    // after we allocated textbox, we have small offset to start texting inside it
    {
        const fl_wrapper::enable_color enable(FL_BLUE);
        drawRectangle(textBoxX, textBoxY, textBoxW, textBoxH);
    }

    const int textOffX = 20;
    const int textOffY = 20;

    switch(floatObj){
        case FOBJ_TILE:
            {
                const fl_wrapper::enable_color enable(FL_RED);

                const int textStartX = textBoxX + textOffX;
                /* */ int textStartY = textBoxY + textOffY;

                drawText(textStartX, textStartY, "     tile");
                textStartY += 20;

                drawText(textStartX, textStartY, "index0 : %d", objFileIndex);
                textStartY += 20;

                drawText(textStartX, textStartY, "index1 : %d", objImageIndex);
                textStartY += 20;

                drawText(textStartX, textStartY, "dbName : %s", g_imageDB->dbName(objFileIndex));
                textStartY += 20;
                break;
            }
        case FOBJ_OBJ0:
        case FOBJ_OBJ1:
            {
                const fl_wrapper::enable_color enable(FL_RED);

                const int textStartX = textBoxX + textOffX;
                /* */ int textStartY = textBoxY + textOffY;

                drawText(textStartX, textStartY, "    obj[%d]", (floatObj == FOBJ_OBJ0) ? 0 : 1);
                textStartY += 20;

                drawText(textStartX, textStartY, "index0 : %d", objFileIndex);
                textStartY += 20;

                drawText(textStartX, textStartY, "index1 : %d", objImageIndex);
                textStartY += 20;

                drawText(textStartX, textStartY, "dbName : %s", g_imageDB->dbName(objFileIndex));
                textStartY += 20;

                const auto &obj = g_editorMap.cell(gridX, gridY).obj[(floatObj == FOBJ_OBJ0) ? 0 : 1];
                drawText(textStartX, textStartY, "alpha  : %s", obj.alpha ? "yes" : "no");
                textStartY += 20;

                drawText(textStartX, textStartY, "animat : %s", obj.animated ? "yes" : "no");
                textStartY += 20;

                drawText(textStartX, textStartY, "tickTy : %d", to_d(obj.tickType));
                textStartY += 20;

                drawText(textStartX, textStartY, "frameC : %d", to_d(obj.frameCount));
                textStartY += 20;

                const auto imgInfo = g_imageDB->setIndex(imageIndex);
                drawText(textStartX, textStartY, "offseX : %d", to_d(imgInfo->px));
                textStartY += 20;

                drawText(textStartX, textStartY, "offseY : %d", to_d(imgInfo->py));
                textStartY += 20;

                drawText(textStartX, textStartY, "Versio : %d", to_d(g_imageDB->getPackage(objFileIndex)->header().version));
                textStartY += 20;
                break;
            }
        default:
            {
                break;
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

/*
 * =====================================================================================
 *
 *       Filename: layerviewarea.cpp
 *        Created: 08/24/2017 15:52:11
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

#include <cmath>
#include <FL/fl_draw.H>
#include "flwrapper.hpp"
#include "editormap.hpp"
#include "imagecache.hpp"
#include "layerviewarea.hpp"
#include "layerviewwindow.hpp"
#include "layerbrowserwindow.hpp"

extern ImageCache g_imageCache;
extern LayerViewWindow *g_layerViewWindow;
extern LayerBrowserWindow *g_layerBrowserWindow;

std::tuple<int, int> LayerViewArea::offset() const
{
    if(!getLayer()){
        return {0, 0};
    }

    const auto [xpCount, ypCount] = getScrollPixelCount();
    const auto [xratio, yratio] = g_layerViewWindow->getScrollBarValue();

    return
    {
        to_d(std::lround(xratio * xpCount)),
        to_d(std::lround(yratio * ypCount)),
    };
}

void LayerViewArea::draw()
{
    BaseArea::draw();
    if(g_layerViewWindow->clearBackground()){
        clear();
    }

    if(getLayer()){
        drawTile();
        drawObject(OBJD_GROUND);
        drawObject(OBJD_OVERGROUND0);
        drawObject(OBJD_OVERGROUND1);
        drawObject(OBJD_SKY);
        drawGrid();
    }
}

int LayerViewArea::handle(int event)
{
    auto result = BaseArea::handle(event);

    // can't find resize event
    // put it here as a hack and check it every time
    g_layerViewWindow->checkScrollBar();

    if(getLayer()){
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
                    g_layerViewWindow->addScrollBarValue(dxratio, dyratio);
                    break;
                }
            case FL_MOUSEWHEEL:
                {
                    const auto [dxratio, dyratio] = getScrollPixelRatio(Fl::event_dx() * SYS_MAPGRIDXP, Fl::event_dy() * SYS_MAPGRIDYP);
                    g_layerViewWindow->addScrollBarValue(dxratio, dyratio);
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
                    const auto [dxratio, dyratio] = getScrollPixelRatio(lastMouseX - m_mouseX, lastMouseY - m_mouseY);
                    g_layerViewWindow->addScrollBarValue(dxratio, dyratio);
                    break;
                }
            case FL_PUSH:
                {
                    fl_cursor(FL_CURSOR_MOVE);
                    result = 1;
                    break;
                }
            default:
                {
                    break;
                }
        }
    }

    g_layerViewWindow->redrawAll();
    return result;
}

void LayerViewArea::drawTile()
{
    if(!g_layerViewWindow->drawTile()){
        return;
    }

    const auto layer = getLayer();
    if(!layer){
        return;
    }

    const auto [offsetX, offsetY] = offset();
    const fl_wrapper::enable_color enable(FL_RED);

    const auto startGX = offsetX / SYS_MAPGRIDXP - 1;
    const auto startGY = offsetY / SYS_MAPGRIDYP - 1;

    const auto drawGW = w() / SYS_MAPGRIDXP + 3;
    const auto drawGH = h() / SYS_MAPGRIDYP + 8;

    for(int y = startGY; y < startGY + drawGH; ++y){
        for(int x = startGX; x < startGX + drawGW; ++x){
            if(!layer->validC(x, y)){
                continue;
            }

            if((x % 2) || (y % 2)){
                continue;
            }

            if(!layer->tile(x, y).valid){
                continue;
            }

            const int startX = x * SYS_MAPGRIDXP - offsetX;
            const int startY = y * SYS_MAPGRIDYP - offsetY;

            if(auto img = g_imageCache.retrieve(layer->tile(x, y).texID)){
                drawImage(img, startX, startY);
                if(g_layerViewWindow->drawTileLine()){
                    drawRectangle(startX, startY, img->w(), img->h());
                }
            }
        }
    }
}

void LayerViewArea::drawGrid()
{
    if(!g_layerViewWindow->drawGridLine()){
        return;
    }

    const auto [offsetX, offsetY] = offset();
    const fl_wrapper::enable_color enable(FL_MAGENTA);

    for(int gx = offsetX / SYS_MAPGRIDXP - 1; gx < (offsetX + w()) / SYS_MAPGRIDXP + 1; ++gx){
        drawLine(gx * SYS_MAPGRIDXP - offsetX, 0, gx * SYS_MAPGRIDXP - offsetX, h());
    }

    for(int gy = offsetY / SYS_MAPGRIDYP - 1; gy < (offsetY + h()) / SYS_MAPGRIDYP + 1; ++gy){
        drawLine(0, gy * SYS_MAPGRIDYP - offsetY, w(), gy * SYS_MAPGRIDYP - offsetY);
    }
}

void LayerViewArea::drawObject(int depth)
{
    if(!g_layerViewWindow->drawObject(depth)){
        return;
    }

    const auto layer = getLayer();
    if(!layer){
        return;
    }

    const auto [offsetX, offsetY] = offset();
    const fl_wrapper::enable_color enable(fl_wrapper::color(depth));

    const int startGX = offsetX / SYS_MAPGRIDXP - 2;
    const int startGY = offsetY / SYS_MAPGRIDYP - 2;

    const int drawGW = w() / SYS_MAPGRIDXP + 4;
    const int drawGH = h() / SYS_MAPGRIDYP + 8;

    for(int y = startGY; y < startGY + drawGH; ++y){
        for(int x = startGX; x < startGX + drawGW; ++x){
            if(!layer->validC(x, y)){
                continue;
            }

            for(const int objIndex: {0, 1}){
                const auto &obj = layer->cell(x, y).obj[objIndex];
                if(!obj.valid){
                    continue;
                }

                if(obj.depth != depth){
                    continue;
                }

                if(auto img = g_imageCache.retrieve(obj.texID)){
                    const int startX = x * SYS_MAPGRIDXP - offsetX;
                    const int startY = y * SYS_MAPGRIDYP + SYS_MAPGRIDYP - img->h() - offsetY;
                    drawImage(img, startX, startY);
                    if(g_layerViewWindow->drawObjectLine(depth)){
                        drawRectangle(startX, startY, img->w(), img->h());
                    }
                }
            }
        }
    }
}

const Mir2xMapData *LayerViewArea::getLayer() const
{
    if(auto layerPtr = g_layerBrowserWindow->getLayer(); layerPtr && layerPtr->valid()){
        return layerPtr;
    }
    return nullptr;
}

std::optional<std::tuple<size_t, size_t>> LayerViewArea::getROISize() const
{
    if(const auto layer = getLayer()){
        return std::tuple<size_t, size_t>(to_uz(layer->w()) * SYS_MAPGRIDXP, to_uz(layer->h()) * SYS_MAPGRIDYP);
    }
    return {};
}

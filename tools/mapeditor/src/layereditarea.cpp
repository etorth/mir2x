/*
 * =====================================================================================
 *
 *       Filename: layereditarea.cpp
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

#include <FL/fl_draw.H>
#include "imagedb.hpp"
#include "editormap.hpp"
#include "imagecache.hpp"
#include "layereditarea.hpp"
#include "layereditorwindow.hpp"
#include "layerbrowserwindow.hpp"

LayerEditArea::LayerEditArea(int nX, int nY, int nW, int nH)
    : BaseArea(nX, nY, nW, nH)
    , m_mouseX(0)
    , m_mouseY(0)
    , m_offsetX(0)
    , m_offsetY(0)
{}

void LayerEditArea::draw()
{
    BaseArea::draw();

    extern LayerEditorWindow *g_layerEditorWindow;
    if(g_layerEditorWindow->ClearBackground()){ Clear(); }

    extern LayerBrowserWindow *g_layerBrowserWindow;
    if(auto pEditorMap = g_layerBrowserWindow->GetLayer()){
        if(pEditorMap->Valid()){
            DrawTile();
            DrawObject(true);
            DrawObject(false);
            DrawGrid();
        }
    }
}

int LayerEditArea::handle(int nEvent)
{
    auto nRet = BaseArea::handle(nEvent);

    extern LayerBrowserWindow *g_layerBrowserWindow;
    if(auto pEditorMap = g_layerBrowserWindow->GetLayer()){
        if(pEditorMap->Valid()){

            int nMouseX = m_mouseX;
            int nMouseY = m_mouseY;
            m_mouseX    = Fl::event_x();
            m_mouseY    = Fl::event_y();

            switch(nEvent){
                case FL_MOUSEWHEEL:
                    {
                        int nDX = Fl::event_dx() * SYS_MAPGRIDXP;
                        int nDY = Fl::event_dy() * SYS_MAPGRIDYP;

                        // don't set scroll bar
                        // layer editor won't support scroll bars
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
                        if(Fl::event_state() & FL_CTRL){
                            // bug of fltk here for windows, when some key is pressed, 
                            // event_x() and event_y() are incorrect!
                        }else{
                            SetOffset(-(m_mouseX - nMouseX), true, -(m_mouseY - nMouseY), true);
                        }

                        break;
                    }
                case FL_PUSH:
                    {
                        fl_cursor(FL_CURSOR_MOVE);

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
    }

    extern LayerEditorWindow *g_layerEditorWindow;
    g_layerEditorWindow->RedrawAll();

    return nRet;
}

void LayerEditArea::DrawTile()
{
    extern LayerEditorWindow *g_layerEditorWindow;
    if(g_layerEditorWindow->DrawTile()){
        extern LayerBrowserWindow *g_layerBrowserWindow;
        if(auto pEditorMap = g_layerBrowserWindow->GetLayer()){
            if(pEditorMap->Valid()){

                int nX = m_offsetX / SYS_MAPGRIDXP - 1;
                int nY = m_offsetY / SYS_MAPGRIDYP - 1;
                int nW = w() / SYS_MAPGRIDXP + 3;
                int nH = h() / SYS_MAPGRIDYP + 8;

                auto fnDraw = [this](uint8_t nFileIndex, uint16_t nImageIndex, int nX, int nY)
                {
                    int nStartX = nX * SYS_MAPGRIDXP - m_offsetX;
                    int nStartY = nY * SYS_MAPGRIDYP - m_offsetY;
                    if(auto pImage = RetrievePNG(nFileIndex, nImageIndex)){
                        DrawImage(pImage, nStartX, nStartY);
                        extern LayerEditorWindow *g_layerEditorWindow;
                        if(g_layerEditorWindow->DrawTileLine()){
                            DrawRectangle(nStartX, nStartY, pImage->w(), pImage->h());
                        }
                    }
                };

                PushColor(FL_RED);
                pEditorMap->DrawTile(nX, nY, nW, nH, fnDraw);
                PopColor();
            }
        }
    }
}

Fl_Image *LayerEditArea::RetrievePNG(uint8_t nFileIndex, uint16_t nImageIndex)
{
    extern ImageDB   *g_imageDB;
    extern ImageCache g_imageCache;
    auto pImage = g_imageCache.Retrieve(nFileIndex, nImageIndex);
    if(pImage == nullptr){
        if(const auto [imgBuf, imgW, imgH] = g_imageDB->decode(nFileIndex, nImageIndex, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF); imgBuf){
            g_imageCache.Register(nFileIndex, nImageIndex, imgBuf, imgW, imgH);
            pImage = g_imageCache.Retrieve(nFileIndex, nImageIndex);
        }
    }
    return pImage;
}

void LayerEditArea::DrawGrid()
{
    extern LayerEditorWindow *g_layerEditorWindow;
    if(g_layerEditorWindow->DrawGridLine()){
        PushColor(FL_MAGENTA);
        for(int nCX = m_offsetX / SYS_MAPGRIDXP - 1; nCX < (m_offsetX + w()) / SYS_MAPGRIDXP + 1; ++nCX){
            DrawLine(nCX * SYS_MAPGRIDXP - m_offsetX, 0, nCX * SYS_MAPGRIDXP - m_offsetX, h());
        }

        for(int nCY = m_offsetY / SYS_MAPGRIDYP - 1; nCY < (m_offsetY + h()) / SYS_MAPGRIDYP + 1; ++nCY){
            DrawLine(0, nCY * SYS_MAPGRIDYP - m_offsetY, w(), nCY * SYS_MAPGRIDYP - m_offsetY);
        }
        PopColor();
    }
}

void LayerEditArea::DrawObject(bool bGround)
{
    extern LayerEditorWindow *g_layerEditorWindow;
    if(g_layerEditorWindow->DrawObject(bGround)){

        extern LayerBrowserWindow *g_layerBrowserWindow;
        if(auto pEditorMap = g_layerBrowserWindow->GetLayer()){
            if(pEditorMap->Valid()){

                PushColor(bGround ? FL_BLUE : FL_GREEN);
                auto fnDrawExt = [this, bGround](int, int)
                {
                };

                auto fnDrawObj = [this, bGround](uint8_t nFileIndex, uint16_t nImageIndex, int nXCnt, int nYCnt)
                {
                    auto pImage = RetrievePNG(nFileIndex, nImageIndex);
                    if(pImage){
                        int nStartX = nXCnt * SYS_MAPGRIDXP - m_offsetX;
                        int nStartY = nYCnt * SYS_MAPGRIDYP + SYS_MAPGRIDYP - pImage->h() - m_offsetY;
                        DrawImage(pImage, nStartX, nStartY);
                        extern LayerEditorWindow *g_layerEditorWindow;
                        if(g_layerEditorWindow->DrawObjectLine(bGround)){
                            DrawRectangle(nStartX, nStartY, pImage->w(), pImage->h());
                        }
                    }
                };

                int nX = m_offsetX / SYS_MAPGRIDXP - 2;
                int nY = m_offsetY / SYS_MAPGRIDYP - 2;
                int nW = w() / SYS_MAPGRIDXP + 4;
                int nH = h() / SYS_MAPGRIDYP + 8;

                pEditorMap->DrawObject(nX, nY, nW, nH, bGround, fnDrawObj, fnDrawExt);
                PopColor();
            }
        }
    }
}

void LayerEditArea::SetOffset(int nX, bool bRelativeX, int nY, bool bRelativeY)
{
    if(auto pEditorMap = GetLayer()){
        if(pEditorMap->Valid()){
            if(bRelativeX){
                m_offsetX += nX;
            }else{
                m_offsetX = nX;
            }
            
            m_offsetX = (std::max<int>)(m_offsetX, -SYS_OBJMAXW * SYS_MAPGRIDXP * 2);
            m_offsetX = (std::min<int>)(m_offsetX, (std::max<int>)(0, SYS_MAPGRIDXP * pEditorMap->W() - w()));

            if(bRelativeY){
                m_offsetY += nY;
            }else{
                m_offsetY = nY;
            }
            m_offsetY = (std::max<int>)(m_offsetY, -SYS_OBJMAXH * SYS_MAPGRIDYP * 2);
            m_offsetY = (std::min<int>)(m_offsetY, (std::max<int>)(0, SYS_MAPGRIDYP * pEditorMap->H() - h()));
        }
    }
}

EditorMap *LayerEditArea::GetLayer()
{
    extern LayerBrowserWindow *g_layerBrowserWindow;
    return g_layerBrowserWindow->GetLayer();
}

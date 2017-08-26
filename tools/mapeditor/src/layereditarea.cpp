/*
 * =====================================================================================
 *
 *       Filename: layereditarea.cpp
 *        Created: 08/24/2017 15:52:11
 *  Last Modified: 08/25/2017 17:03:21
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

#include <FL/fl_draw.H>
#include "imagedb.hpp"
#include "editormap.hpp"
#include "imagecache.hpp"
#include "layereditarea.hpp"
#include "layereditorwindow.hpp"
#include "layerbrowserwindow.hpp"

LayerEditArea::LayerEditArea(int nX, int nY, int nW, int nH)
    : BaseArea(nX, nY, nW, nH)
    , m_MouseX(0)
    , m_MouseY(0)
    , m_OffsetX(0)
    , m_OffsetY(0)
{}

void LayerEditArea::draw()
{
    BaseArea::draw();

    extern LayerEditorWindow *g_LayerEditorWindow;
    if(g_LayerEditorWindow->ClearBackground()){ Clear(); }

    extern LayerBrowserWindow *g_LayerBrowserWindow;
    if(auto pEditorMap = g_LayerBrowserWindow->GetLayer()){
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

    extern LayerBrowserWindow *g_LayerBrowserWindow;
    if(auto pEditorMap = g_LayerBrowserWindow->GetLayer()){
        if(pEditorMap->Valid()){

            int nMouseX = m_MouseX;
            int nMouseY = m_MouseY;
            m_MouseX    = Fl::event_x();
            m_MouseY    = Fl::event_y();

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
                            SetOffset(-(m_MouseX - nMouseX), true, -(m_MouseY - nMouseY), true);
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

    extern LayerEditorWindow *g_LayerEditorWindow;
    g_LayerEditorWindow->RedrawAll();

    return nRet;
}

void LayerEditArea::DrawTile()
{
    extern LayerEditorWindow *g_LayerEditorWindow;
    if(g_LayerEditorWindow->DrawTile()){
        extern LayerBrowserWindow *g_LayerBrowserWindow;
        if(auto pEditorMap = g_LayerBrowserWindow->GetLayer()){
            if(pEditorMap->Valid()){

                int nX = m_OffsetX / SYS_MAPGRIDXP - 1;
                int nY = m_OffsetY / SYS_MAPGRIDYP - 1;
                int nW = w() / SYS_MAPGRIDXP + 3;
                int nH = h() / SYS_MAPGRIDYP + 8;

                auto fnDraw = [this](uint8_t nFileIndex, uint16_t nImageIndex, int nX, int nY)
                {
                    int nStartX = nX * SYS_MAPGRIDXP - m_OffsetX;
                    int nStartY = nY * SYS_MAPGRIDYP - m_OffsetY;
                    if(auto pImage = RetrievePNG(nFileIndex, nImageIndex)){
                        DrawImage(pImage, nStartX, nStartY);
                        extern LayerEditorWindow *g_LayerEditorWindow;
                        if(g_LayerEditorWindow->DrawTileLine()){
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

void LayerEditArea::DrawGrid()
{
    extern LayerEditorWindow *g_LayerEditorWindow;
    if(g_LayerEditorWindow->DrawGridLine()){
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

void LayerEditArea::DrawObject(bool bGround)
{
    extern LayerEditorWindow *g_LayerEditorWindow;
    if(g_LayerEditorWindow->DrawObject(bGround)){

        extern LayerBrowserWindow *g_LayerBrowserWindow;
        if(auto pEditorMap = g_LayerBrowserWindow->GetLayer()){
            if(pEditorMap->Valid()){

                PushColor(bGround ? FL_BLUE : FL_GREEN);
                auto fnDrawExt = [this, bGround](int, int)
                {
                };

                auto fnDrawObj = [this, bGround](uint8_t nFileIndex, uint16_t nImageIndex, int nXCnt, int nYCnt)
                {
                    auto pImage = RetrievePNG(nFileIndex, nImageIndex);
                    if(pImage){
                        int nStartX = nXCnt * SYS_MAPGRIDXP - m_OffsetX;
                        int nStartY = nYCnt * SYS_MAPGRIDYP + SYS_MAPGRIDYP - pImage->h() - m_OffsetY;
                        DrawImage(pImage, nStartX, nStartY);
                        extern LayerEditorWindow *g_LayerEditorWindow;
                        if(g_LayerEditorWindow->DrawObjectLine(bGround)){
                            DrawRectangle(nStartX, nStartY, pImage->w(), pImage->h());
                        }
                    }
                };

                int nX = m_OffsetX / SYS_MAPGRIDXP - 2;
                int nY = m_OffsetY / SYS_MAPGRIDYP - 2;
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
                m_OffsetX += nX;
            }else{
                m_OffsetX = nX;
            }
            
            m_OffsetX = (std::max)(m_OffsetX, -SYS_OBJMAXW * SYS_MAPGRIDXP * 2);
            m_OffsetX = (std::min)(m_OffsetX, (std::max)(0, SYS_MAPGRIDXP * pEditorMap->W() - w()));

            if(bRelativeY){
                m_OffsetY += nY;
            }else{
                m_OffsetY = nY;
            }
            m_OffsetY = (std::max)(m_OffsetY, -SYS_OBJMAXH * SYS_MAPGRIDYP * 2);
            m_OffsetY = (std::min)(m_OffsetY, (std::max)(0, SYS_MAPGRIDYP * pEditorMap->H() - h()));
        }
    }
}

EditorMap *LayerEditArea::GetLayer()
{
    extern LayerBrowserWindow *g_LayerBrowserWindow;
    return g_LayerBrowserWindow->GetLayer();
}

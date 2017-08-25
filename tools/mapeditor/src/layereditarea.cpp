/*
 * =====================================================================================
 *
 *       Filename: layereditarea.cpp
 *        Created: 08/24/2017 15:52:11
 *  Last Modified: 08/25/2017 14:22:50
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

#include "layereditarea.hpp"
#include "layereditorwindow.hpp"

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
}

int LayerEditArea::handle(int nEvent)
{
    auto nRet = BaseArea::handle(nEvent);

    extern LayerEditorWindow *g_LayerEditorWindow;
    g_LayerEditorWindow->RedrawAll();

    return nRet;
}

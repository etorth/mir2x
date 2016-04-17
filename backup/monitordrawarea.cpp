/*
 * =====================================================================================
 *
 *       Filename: monitordrawarea.cpp
 *        Created: 04/14/2016 04:27:57 AM
 *  Last Modified: 04/14/2016 22:44:43
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

#include <cstring>
#include "supwarning.hpp"
#include "drawarea.hpp"
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_RGB_Image.H>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <vector>
#include <string>
#include "mir2map.hpp"
#include <algorithm>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <functional>
#include "mainwindow.hpp"
#include "attributeselectwindow.hpp"
#include "mathfunc.hpp"
#include "editormap.hpp"

#include "imagedb.hpp"
#include "imagecache.hpp"

// define 256 colors for use, then we won't be short of color any more
// make is global only in current file
static uint32_t g_Color[] = {
    0X80FF0000, 0X8000FF00, 0X800000FF, 0X8000FFFF, 0X80FF00FF, 0X80FFFF00, 0X8069C667, 0X80FF5173,
    0X8029EC4A, 0X80ABBACD, 0X80E3FBF2, 0X80C27C46, 0X801BF854, 0X808DE7E8, 0X802E5A76, 0X809F3363,
    0X80669AC9, 0X80B70D32, 0X80A35831, 0X805D255A, 0X80581705, 0X80D45EE9, 0X80CDB2AB, 0X80B49BC6,
    0X800E1154, 0X80417482, 0X80DC3D21, 0X80E97087, 0X8041A13E, 0X8067FCE1, 0X807E013E, 0X80DCEA97,
    0X808F966B, 0X802A5C38, 0X803BB0EC, 0X80AF32FB, 0X80EC543C, 0X805CDB18, 0X80FE1A02, 0X80FAFB43,
    0X80FB3AAA, 0X80E6D129, 0X807C3C05, 0X80D87594, 0X808961BE, 0X80BB5CF9, 0X800F99A8, 0X80EBB195,
    0X8005B3F1, 0X8000F7EF, 0X803AA1E9, 0X800BCAE5, 0X8048D0CB, 0X80BD6447, 0X801E231F, 0X807B1CA8,
    0X8014C564, 0X80C55A73, 0X80794B5E, 0X80703B63, 0X80112464, 0X80DC099E, 0X80ACD4AA, 0X80101BF2,
    0X80333BAF, 0X8050E3CD, 0X80154748, 0X806FBB5C, 0X80BA1922, 0X80F57D9B, 0X801AE10B, 0X80237F1C,
    0X80F829F8, 0X80131BA4, 0X804ECAB5, 0X803298E8, 0X8079E038, 0X80343D4D, 0X804E5FBC, 0X80CBFA77,
    0X80AC056C, 0X802B2186, 0X80551AAA, 0X8070BEA2, 0X803B73B5, 0X80D35C04, 0X80B39436, 0X80F0E2AF,
    0X804F9EE4, 0X80491532, 0X804E82FD, 0X807008A9, 0X808AB2D4, 0X80485429, 0X80BC0A9A, 0X80180ED5,
    0X80AC44A8, 0X808EF35B, 0X802DD74C, 0X8042099B, 0X80C406E5, 0X80CDAF33, 0X807F84A3, 0X80D4AD2D,
    0X80DE4776, 0X80EC1C32, 0X8030C44A, 0X802320F6, 0X80FB6C85, 0X800407B2, 0X800BECF4, 0X80BA20B9,
    0X803EC386, 0X80ECF105, 0X803367D9, 0X805099B7, 0X8014E3A3, 0X8034D9D3, 0X80A05EF7, 0X80A810F2,
    0X809405F6, 0X80B4BE01, 0X807844BC, 0X806949FA, 0X80D023E6, 0X8069DA1A, 0X804C7E6A, 0X8025517E,
    0X808448B3, 0X80943A53, 0X809931FB, 0X80573290, 0X809BEE44, 0X80E5E9BC, 0X8008CF25, 0X80E2E9F5,
    0X8060535E, 0X80B2D2AA, 0X80FA85D0, 0X8035D854, 0X8066D4E8, 0X80986482, 0X8087A8D9, 0X80706575,
    0X803F8A5A, 0X80298062, 0X807CDE44, 0X804E89A5, 0X80D35957, 0X80ACAD51, 0X80809586, 0X80E417EC,
    0X808CF185, 0X80F1660C, 0X807CC07C, 0X80FC22BB, 0X80DA66E4, 0X80630B61, 0X80BC62AF, 0X8069B483,
    0X80FF3A2F, 0X801627AF, 0X8007AC93, 0X806DB81F, 0X802D3411, 0X804FEF8D, 0X80B6D489, 0X80C13563,
    0X8024E4C7, 0X80D86783, 0X801296ED, 0X803945EC, 0X80E5D802, 0X809DF80A, 0X80D10977, 0X80C196A5,
    0X80951FF4, 0X80CA82AA, 0X80AE496C, 0X8016CD90, 0X80ACBA68, 0X80F2A67A, 0X80CAA8B4, 0X80C2B299,
    0X80CB2A37, 0X8061CF08, 0X8080C3C9, 0X80036E5E, 0X804CDA28, 0X80196AD7, 0X80D3D2ED, 0X80794C99,
    0X8022008B, 0X80D49A56, 0X80FED118, 0X80CDD9E4, 0X8091A345, 0X80FF01C6, 0X80D92AC9, 0X80430115,
    0X8015EE2F, 0X80618702, 0X8062137C, 0X80FC699E, 0X80CD8172, 0X80A66571, 0X8049AB3E, 0X804B71CF,
    0X80753ACE, 0X80764FA7, 0X80647EEA, 0X80EB81FF, 0X80FEFD61, 0X80679BC3, 0X80E90DBF, 0X804E7E8C,
    0X80F9BD32, 0X806A8C7C, 0X80A45BC7, 0X80F4023C, 0X8072EDB2, 0X80F3EC16, 0X80F04D01, 0X808B1000,
    0X8099CF67, 0X80175B50, 0X80D48E9F, 0X80610A98, 0X80BCD103, 0X80BE0DA7, 0X80ABBF9B, 0X8098D50E,
    0X80E5D601, 0X80F6D6F2, 0X80C53E7D, 0X80218E16, 0X80AF2D2E, 0X80B9C602, 0X808AC963, 0X8097701F,
    0X80560CDE, 0X802B1A89, 0X80011B21, 0X80D80D07, 0X80168BFD, 0X80A4A1C2, 0X80D2CFE3, 0X8098D292,
    0X8061354B, 0X80D155D5, 0X80DD336C, 0X80F7BCC2, 0X8013DEED, 0X8020E5EF, 0X80ABE2C7, 0X804DA4DD
};

MonitorDrawArea::MonitorDrawArea(int x, int y, int w, int h)
    : Fl_Box(x, y, w, h)
    , m_MouseX(0)
    , m_MouseY(0)
    , m_OffsetX(0)
      , m_OffsetY(0)
      , m_TUC{{nullptr, nullptr, nullptr, nullptr}, {nullptr, nullptr, nullptr, nullptr}}
    , m_TextBoxBG(nullptr)
, m_LightUC(nullptr)
{
    m_TUC[0][0] = CreateTUC(0, true);
    m_TUC[0][1] = CreateTUC(1, true);
    m_TUC[0][2] = CreateTUC(2, true);
    m_TUC[0][3] = CreateTUC(3, true);

    m_TUC[1][0] = CreateTUC(0, false);
    m_TUC[1][1] = CreateTUC(1, false);
    m_TUC[1][2] = CreateTUC(2, false);
    m_TUC[1][3] = CreateTUC(3, false);

    CreateRC();
}

MonitorDrawArea::~MonitorDrawArea()
{
    delete m_LightUC;
    delete m_TextBoxBG;
    delete m_TUC[0][0];
    delete m_TUC[0][1];
    delete m_TUC[0][2];
    delete m_TUC[0][3];
    delete m_TUC[1][0];
    delete m_TUC[1][1];
    delete m_TUC[1][2];
    delete m_TUC[1][3];
}

void MonitorDrawArea::draw()
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
    DrawObject(true);
    DrawObject(false);

    DrawLight();

    DrawAttributeGrid();
    DrawGrid();

    DrawSelect();
    DrawTrySelect();
    DrawTextBox();
}

void MonitorDrawArea::DrawSelectBySingle()
{
    int nX, nY, nIndex;
    if(LocateGroundSubCell( m_MouseX - x() + m_OffsetX,
                m_MouseY - y() + m_OffsetY, nX, nY, nIndex)){
        extern MainWindow *g_MainWindow;
        DrawTUC(nX, nY, nIndex, !g_MainWindow->Deselect());
    }
}

void MonitorDrawArea::AddSelectBySingle()
{
    int nX, nY, nIndex;
    if(LocateGroundSubCell( m_MouseX - x() + m_OffsetX,
                m_MouseY - y() + m_OffsetY, nX, nY, nIndex)){
        extern EditorMap g_EditorMap;
        extern MainWindow *g_MainWindow;
        g_EditorMap.SetGroundSelect(nX, nY, nIndex, g_MainWindow->Deselect() ? 0 : 1);
    }
}

void MonitorDrawArea::RhombusCoverOperation(int nMX, int nMY, int nSize,
        std::function<void(int, int, int)> fnOperation)
{
    if(nSize <= 0){ return; }

    // don't check boundary condition
    // since even center point is out of map
    // we can still select grids over map

    int nCX = nMX / 48;
    int nCY = nMY / 32 - nSize / 2;

    // mode 0: 0, 2
    // mode 1: 1, 3

    int nStartMode = (2 * (nMX % 48) < 3 * (nMY % 32) ) ? 0 : 1;
    int nMode = nStartMode;
    int nLine = 1;

    auto fnLineCoverInfo = [nCX, nCY, nStartMode](
            int &nStartX, int &nStartY, int &nCnt, int nLine){
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
                fnOperation(nStartX + nIndex, nStartY    , 2);
                fnOperation(nStartX + nIndex, nStartY + 1, 0);
            }else{
                fnOperation(nStartX + nIndex    , nStartY, 1);
                fnOperation(nStartX + nIndex + 1, nStartY, 3);
            }
        }
        nMode = 1 - nMode;
        nLine++;
    }
}

void MonitorDrawArea::DrawSelectByRhombus()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->RhombusSize();
    int nMX   = m_MouseX + m_OffsetX - x();
    int nMY   = m_MouseY + m_OffsetY - y();

    auto fnDraw = [this](int nX,  int nY, int nIndex){
        extern MainWindow *g_MainWindow;
        DrawTUC(nX, nY, nIndex, !g_MainWindow->Deselect());
    };

    RhombusCoverOperation(nMX, nMY, nSize, fnDraw);
}

void MonitorDrawArea::AddSelectByRhombus()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->RhombusSize();
    int nMX   = m_MouseX + m_OffsetX - x();
    int nMY   = m_MouseY + m_OffsetY - y();

    auto fnSet = [](int nX,  int nY, int nIndex){
        extern EditorMap g_EditorMap;
        extern MainWindow *g_MainWindow;
        g_EditorMap.SetGroundSelect(nX, nY, nIndex, g_MainWindow->Deselect() ? 0 : 1);
    };

    RhombusCoverOperation(nMX, nMY, nSize, fnSet);
}

void MonitorDrawArea::RectangleCoverOperation(
        int nMouseXOnMap, int nMouseYOnMap, int nSize, std::function<void(int, int, int)> fnOperation)
{
    if(nSize <= 0){ return; }

    int nMX = nMouseXOnMap / 48 - nSize / 2;
    int nMY = nMouseYOnMap / 32 - nSize / 2;

    for(int nX = 0; nX < nSize; ++nX){
        for(int nY = 0; nY < nSize; ++nY){
            for(int nIndex = 0; nIndex < 4; ++nIndex){
                fnOperation(nX + nMX, nY + nMY, nIndex);
            }
        }
    }
}

void MonitorDrawArea::DrawSelectByRectangle()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->RectangleSize();
    int nMX   = m_MouseX + m_OffsetX - x();
    int nMY   = m_MouseY + m_OffsetY - y();

    auto fnDraw = [this](int nX,  int nY, int nIndex){
        extern MainWindow *g_MainWindow;
        DrawTUC(nX, nY, nIndex, !g_MainWindow->Deselect());
    };

    RectangleCoverOperation(nMX, nMY, nSize, fnDraw);
}

void MonitorDrawArea::AddSelectByRectangle()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->RectangleSize();
    int nMX   = m_MouseX + m_OffsetX - x();
    int nMY   = m_MouseY + m_OffsetY - y();

    auto fnSet = [](int nX,  int nY, int nIndex){
        extern EditorMap g_EditorMap;
        extern MainWindow *g_MainWindow;
        g_EditorMap.SetGroundSelect(nX, nY, nIndex, g_MainWindow->Deselect() ? 0 : 1);
    };

    RectangleCoverOperation(nMX, nMY, nSize, fnSet);
}

void MonitorDrawArea::DrawSelectByAttribute()
{
    extern SelectSettingWindow *g_SelectSettingWindow;
    int nSize = g_SelectSettingWindow->AttributeSize();

    if(nSize <= 0){ return; }

    int nMX = m_MouseX + m_OffsetX - x();
    int nMY = m_MouseY + m_OffsetY - y();

    auto fnDraw = [this](int nX,  int nY, int nIndex){
        extern AttributeSelectWindow *g_AttributeSelectWindow;
        extern MainWindow *g_MainWindow;
        extern EditorMap g_EditorMap;
        bool bValid = g_EditorMap.GroundValid(nX, nY, nIndex);
        uint8_t nValue = (bValid ? g_EditorMap.Ground(nX, nY, nIndex) : 0);
        if(g_AttributeSelectWindow->Test(bValid, nValue)){
            DrawTUC(nX, nY, nIndex, !g_MainWindow->Deselect());
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

    auto wColor = fl_color();
    fl_color(FL_YELLOW);
    DrawRectangle(nAX, nAY, nAW, nAH);
    fl_color(wColor);
}

void MonitorDrawArea::AddSelectByAttribute()
{
    extern SelectSettingWindow *g_SelectSettingWindow;

    int nSize = g_SelectSettingWindow->AttributeSize();
    int nMX   = m_MouseX + m_OffsetX - x();
    int nMY   = m_MouseY + m_OffsetY - y();

    auto fnSet = [](int nX,  int nY, int nIndex){
        extern MainWindow *g_MainWindow;
        extern EditorMap g_EditorMap;
        bool bValid = g_EditorMap.GroundValid(nX, nY, nIndex);
        uint8_t nValue = (bValid ? g_EditorMap.Ground(nX, nY, nIndex) : 0);

        // TODO think about it
        extern AttributeSelectWindow *g_AttributeSelectWindow;
        if(g_AttributeSelectWindow->Test(bValid, nValue)){
            g_EditorMap.SetGroundSelect(nX, nY, nIndex, g_MainWindow->Deselect() ? 0 : 1);
        }
    };
    AttributeCoverOperation(nMX, nMY, nSize, fnSet);
}

void MonitorDrawArea::DrawSelect()
{
    extern EditorMap g_EditorMap;

    int nX = m_OffsetX / 48 - 1;
    int nY = m_OffsetY / 32 - 1;

    int nXSize = w() / 48 + 3;
    int nYSize = h() / 32 + 3;

    for(int nTX = nX; nTX < nXSize + nX; ++nTX){
        for(int nTY = nY; nTY < nYSize + nY; ++nTY){
            if(g_EditorMap.ValidC(nTX, nTY)){
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    extern MainWindow *g_MainWindow;
                    if(g_EditorMap.GroundSelect(nTX, nTY, nIndex) == !g_MainWindow->Reversed()){
                        DrawTUC(nTX, nTY, nIndex, !g_MainWindow->Reversed());
                    }
                }
            }
        }
    }
}

void MonitorDrawArea::DrawTrySelect()
{
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->EnableSelect()){
        auto wColor = fl_color();
        fl_color(FL_RED);

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

        fl_color(wColor);
    }
}

void MonitorDrawArea::DrawTextBox()
{
    if(m_TextBoxBG == nullptr){
        uint32_t pData[32 * 5][48 * 4];
        for(int nY = 0; nY < 32 * 5; ++nY){
            for(int nX = 0; nX < 48 * 4; ++nX){
                pData[nY][nX] = 0X80000000;
            }
        }
        m_TextBoxBG = Fl_RGB_Image((uchar *)pData, 48 * 4, 32 * 5, 4, 0).copy(48 * 4, 32 * 5);
    }

    DrawImage(m_TextBoxBG, 0, 0);

    auto wColor = fl_color();
    fl_color(FL_RED);

    char szInfo[128];
    std::sprintf(szInfo, "OffsetX: %d %d", m_OffsetX / 48, m_OffsetX);
    fl_draw(szInfo, 10 + x(), 20 + y());

    std::sprintf(szInfo, "OffsetY: %d %d", m_OffsetY / 32, m_OffsetY);
    fl_draw(szInfo, 10 + x(), 40 + y());

    int nMX = std::max(0, m_MouseX + m_OffsetX - x());
    int nMY = std::max(0, m_MouseY + m_OffsetY - y());

    std::sprintf(szInfo, "MouseMX: %d %d", nMX / 48, nMX);
    fl_draw(szInfo, 10 + x(), 60 + y());

    std::sprintf(szInfo, "MouseMY: %d %d", nMY / 32, nMY);
    fl_draw(szInfo, 10 + x(), 80 + y());

    fl_color(wColor);
}

void MonitorDrawArea::DrawObject(bool bGround)
{
    extern MainWindow *g_MainWindow;
    auto wColor = fl_color();
    if(bGround){
        if(!g_MainWindow->ShowGroundObjectLayer()){ return; }
        fl_color(FL_BLUE);
    }else{
        if(!g_MainWindow->ShowOverGroundObjectLayer()){ return; }
        fl_color(FL_GREEN);
    }

    auto fnDrawObj = [this, bGround](uint8_t nFileIndex, uint16_t nImageIndex, int nXCnt, int nYCnt){
        auto p = RetrievePNG(nFileIndex, nImageIndex);
        if(p){
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
                if(g_MainWindow->ShowOverGroundObjectLine()){
                    DrawRectangle(nStartX, nStartY, p->w(), p->h());
                }
            }
        }
    };

    extern EditorMap g_EditorMap;
    g_EditorMap.DrawObject(
            m_OffsetX / 48 - 10, m_OffsetY / 32 - 20, w() / 48 + 20, h() / 32 + 40, bGround, fnDrawObj);

    fl_color(wColor);
}

// coordinate (0, 0) is on most top-left of *MonitorDrawArea*
// not for map or window
void MonitorDrawArea::DrawImage(Fl_Image *pImage, int nAX, int nAY)
{
    int nX = nAX + x();
    int nY = nAY + y();
    int nW = pImage->w();
    int nH = pImage->h();

    if(pImage == nullptr
            || nAX >= w() || nAX + pImage->w() <= 0
            || nAY >= h() || nAY + pImage->h() <= 0){
        return;
    }

    int nSX = 0;
    int nSY = 0;

    if(nAX < 0){
        nSX -= nAX;
        nW  += nAX;
        nX   = x();
    }

    if(nAX + pImage->w() > w()){
        nW = w() - nAX;
    }

    if(nAY < 0){
        nSY -= nAY;
        nH  += nAY;
        nY   = y();
    }

    if(nAY + pImage->h() > h()){
        nH = h() - nAY;
    }

    pImage->draw(nX, nY, nW, nH, nSX, nSY);
}

void MonitorDrawArea::DrawAttributeGrid()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowAttributeGridLine()){ return; }

    auto wColor = fl_color();
    fl_color(FL_MAGENTA);

    for(int nCX = m_OffsetX / 48 - 1; nCX < (m_OffsetX + w()) / 48 + 1; ++nCX){
        for(int nCY = m_OffsetY / 32 - 1; nCY < (m_OffsetY + h()) / 32 + 1; ++nCY){
            for(int nIndex = 0; nIndex < 4; ++nIndex){
                extern EditorMap g_EditorMap;
                if(g_EditorMap.ValidC(nCX, nCY)){
                    bool    bValid = (g_EditorMap.GroundValid(nCX, nCY, nIndex) != 0);
                    uint8_t nValue = (g_EditorMap.Ground(nCX, nCY, nIndex));

                    extern AttributeGridWindow *g_AttributeGridWindow;
                    if(g_AttributeGridWindow->Test(bValid, nValue)){
                        int nMidX, nMidY, nX1, nY1, nX2, nY2;
                        GetTriangleOnMap(nCX, nCY, nIndex, nMidX, nMidY, nX1, nY1, nX2, nY2);
                        // TODO
                        DrawLoop(nMidX - m_OffsetX, nMidY - m_OffsetY,
                                nX1 - m_OffsetX, nY1 - m_OffsetY, nX2 - m_OffsetX, nY2 - m_OffsetY);
                    }
                }
            }
        }
    }

    fl_color(wColor);
}

void MonitorDrawArea::DrawGrid()
{
    // this function draw all grids for assistance
    // for selection purpose, use selection operation
    //------->   0 
    //-------> 3   1
    //------->   2

    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowGridLine()){ return; }

    int nDX = x() - m_OffsetX;
    int nDY = y() - m_OffsetY;

    auto wColor = fl_color();
    fl_color(FL_MAGENTA);

    for(int nCX = m_OffsetX / 48 - 1; nCX < (m_OffsetX + w()) / 48 + 1; ++nCX){
        for(int nCY = m_OffsetY / 32 - 1; nCY < (m_OffsetY + h()) / 32 + 1; ++nCY){
            for(int nIndex = 0; nIndex < 4; ++nIndex){
                extern EditorMap g_EditorMap;
                if(g_EditorMap.ValidC(nCX, nCY)){
                    int nMidX, nMidY, nX1, nY1, nX2, nY2;
                    GetTriangleOnMap(nCX, nCY, nIndex, nMidX, nMidY, nX1, nY1, nX2, nY2);
                    // TODO
                    fl_loop(nMidX + nDX, nMidY + nDY, nX1 + nDX, nY1 + nDY, nX2 + nDX, nY2 + nDY);
                }
            }
        }
    }

    fl_color(wColor);
}

Fl_Image *MonitorDrawArea::RetrievePNG(uint8_t nFileIndex, uint16_t nImageIndex)
{
    extern ImageDB    g_ImageDB;
    extern ImageCache g_ImageCache;
    auto p = g_ImageCache.Retrieve(nFileIndex, nImageIndex);
    if(p == nullptr){
        if(g_ImageDB.Valid(nFileIndex, nImageIndex)){
            int nW = g_ImageDB.FastW(nFileIndex);
            int nH = g_ImageDB.FastH(nFileIndex);
            g_ImageCache.Register(nFileIndex, nImageIndex, 
                    g_ImageDB.FastDecode(nFileIndex, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF), nW, nH);
            p = g_ImageCache.Retrieve(nFileIndex, nImageIndex);
        }
    }
    return p;
}

void MonitorDrawArea::DrawTile()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowBaseTileLayer()){ return; }

    auto wColor = fl_color();
    fl_color(FL_RED);

    auto fnDraw = [this](uint8_t nFileIndex, uint16_t nImageIndex, int nX, int nY){
        int nStartX = nX * 48 - m_OffsetX;
        int nStartY = nY * 32 - m_OffsetY;
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
    g_EditorMap.DrawTile(
            m_OffsetX / 48 - 5, m_OffsetY / 32 - 5, w() / 48 + 10, h() / 32 + 10, fnDraw);

    fl_color(wColor);
}

void MonitorDrawArea::SetOffset(int nX, bool bRelativeX, int nY, bool bRelativeY)
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
    m_OffsetY  = (std::max)(m_OffsetY, 0);
    m_OffsetY  = (std::min)(m_OffsetY, (std::max)(0, 32 * g_EditorMap.H() - h()));
}

int MonitorDrawArea::handle(int nEvent)
{
    int ret = Fl_Box::handle(nEvent);

    extern EditorMap g_EditorMap;
    if(!g_EditorMap.Valid()){
        return ret;
    }

    int mouseX = m_MouseX;
    int mouseY = m_MouseY;
    m_MouseX   = Fl::event_x();
    m_MouseY   = Fl::event_y();

    switch(nEvent){
        case FL_RELEASE:
            fl_cursor(FL_CURSOR_DEFAULT);
            break;

        case FL_MOVE:
            break;

        case FL_DRAG:
            {
                extern MainWindow *g_MainWindow;
                if(g_MainWindow->EnableSelect()){
                    AddSelect();
                }else if(g_MainWindow->EnableEdit()){
                    // TODO
                    //
                }else{
                    if(Fl::event_state() & FL_CTRL){
                        // bug of fltk here for windows, when some key is pressed, 
                        // event_x() and event_y() are incorrect!
                    }else{
                        // SetOffset(-(m_MouseX - mouseX), true, -(m_MouseY - mouseY), true);
                        SetOffset(-(m_MouseX - mouseX), true, -(m_MouseY - mouseY), true);
                        SetScrollBar();
                    }
                }
            }
            break;

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
            }
            // for drag nEvent
            ret = 1;
            break;
        default:
            break;
    }

    extern MainWindow *g_MainWindow;
    g_MainWindow->RedrawAll();

    return ret;
}

bool MonitorDrawArea::LocateGroundSubCell(int nXOnMap, int nYOnMap, int &nX, int &nY, int &nIndex)
{
    extern EditorMap g_EditorMap;
    if(!g_EditorMap.ValidP(nXOnMap, nYOnMap)){ return false; }

    nX = nXOnMap / 48;
    nY = nYOnMap / 32;

    for(int nIndexLoop = 0; nIndexLoop < 4; ++nIndexLoop){
        int nMidX, nMidY, nX1, nY1, nX2, nY2;
        GetTriangleOnMap(nX, nY, nIndexLoop, nMidX, nMidY, nX1, nY1, nX2, nY2);
        if(PointInTriangle(nXOnMap, nYOnMap, nMidX, nMidY, nX1, nY1, nX2, nY2)){
            nIndex = nIndexLoop;
            return true;
        }
    }
    return false;
}

void MonitorDrawArea::GetTriangleOnMap(
        int nCellX, int nCellY, int nIndex, // ientfy cell
        int &nX0, int &nY0,  // 0
        int &nX1, int &nY1,  // 1
        int &nX2, int &nY2)  // 2
{
    // for triangle index in a grid:
    //------->   0 
    //-------> 3   1
    //------->   2
    //
    //for points in a triangle: use clock-wise:
    //  1
    // /|
    //0 |    1-------2
    // \|     \     /
    //  2      \   /
    //          \ /
    //           0

    // int nStartX = nX * 48 + x() - m_OffsetX;
    // int nStartY = nY * 32 + y() - m_OffsetY;
    int nStartX = nCellX * 48;
    int nStartY = nCellY * 32;
    int nStopX  = nStartX + 48;
    int nStopY  = nStartY + 32;

    nX0 = (nStartX + nStopX) / 2;
    nY0 = (nStartY + nStopY) / 2;

    switch(nIndex % 4){
        case 0:
            nX1 = nStartX;
            nY1 = nStartY;
            nX2 = nStopX;
            nY2 = nStartY;
            break;
        case 1:
            nX1 = nStopX;
            nY1 = nStartY;
            nX2 = nStopX;
            nY2 = nStopY;
            break;
        case 2:
            nX1 = nStopX;
            nY1 = nStopY;
            nX2 = nStartX;
            nY2 = nStopY;
            break;
        case 3:
            nX1 = nStartX;
            nY1 = nStopY;
            nX2 = nStartX;
            nY2 = nStartY;
            break;
        default:
            break;
    }
}

Fl_Image *MonitorDrawArea::CreateTUC(int nIndex, bool bSelect)
{
    uint32_t nCB = (0X00000000);
    uint32_t nCF = (bSelect ? 0X800000FF : 0X8000FF00);
    uint32_t pData[32][48];
    for(int nY = 0; nY < 32; ++nY){
        for(int nX = 0; nX < 48; ++nX){
            switch(nIndex % 4){
                case 0:
                    if(3 * (16 - nY) >= 2 * std::abs(nX - 24)){
                        pData[nY][nX] = nCF;
                    }else{
                        pData[nY][nX] = nCB;
                    }
                    break;
                case 1:
                    if(3 * std::abs(nY - 16) <= 2 * (nX - 24)){
                        pData[nY][nX] = nCF;
                    }else{
                        pData[nY][nX] = nCB;
                    }
                    break;
                case 2:
                    if(3 * (nY - 16) >= 2 * std::abs(nX - 24)){
                        pData[nY][nX] = nCF;
                    }else{
                        pData[nY][nX] = nCB;
                    }
                    break;
                case 3:
                    if(3 * std::abs(nY - 16) <= 2 * (24 - nX)){
                        pData[nY][nX] = nCF;
                    }else{
                        pData[nY][nX] = nCB;
                    }
                    break;
                default:
                    pData[nY][nX] = nCB;
                    break;
            }
        }
    }
    return Fl_RGB_Image((uchar *)(pData), 48, 32, 4, 0).copy(48, 32);
}

void MonitorDrawArea::DrawTUC(int nCX, int nCY, int nIndex, bool bSelect)
{
    extern EditorMap g_EditorMap;
    if(g_EditorMap.Valid() && g_EditorMap.ValidC(nCX, nCY)){
        DrawImage(m_TUC[bSelect ? 0 : 1][nIndex % 4], nCX * 48 - m_OffsetX, nCY * 32 - m_OffsetY);
    }
}

void MonitorDrawArea::ClearGroundSelect()
{
    extern EditorMap g_EditorMap;
    if(!g_EditorMap.Valid()){
        return;
    }

    g_EditorMap.ClearGroundSelect();
}

void MonitorDrawArea::SetScrollBar()
{
    extern EditorMap g_EditorMap;
    double fXP = -1.0;
    double fYP = -1.0;
    if(48 * g_EditorMap.W()  - w() > 0){
        fXP = m_OffsetX * 1.0 / (48 * g_EditorMap.W()  - w());
    }
    if(32 * g_EditorMap.H() - h() > 0){
        fYP = m_OffsetY * 1.0 / (32 * g_EditorMap.H() - h());
    }

    extern MainWindow *g_MainWindow;
    g_MainWindow->UpdateScrollBar(fXP, fYP);
}

void MonitorDrawArea::AddSelect()
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

// truncate line into segment inside *MonitorDrawArea*
bool MonitorDrawArea::LocateLineSegment(int &nX1, int &nY1, int &nX2, int &nY2)
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

    auto fnClipT = [](int nNum, int nDenom, double &ftE, double &ftL){
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

// coordinate (0, 0) is on most top-left of *MonitorDrawArea*
void MonitorDrawArea::DrawLine(int nAX0, int nAY0, int nAX1, int nAY1)
{
    if(LocateLineSegment(nAX0, nAY0, nAX1, nAY1)){
        fl_line(nAX0 + x(), nAY0 + y(), nAX1 + x(), nAY1 + y());
    }
}

void MonitorDrawArea::DrawRectangle(int nAX, int nAY, int nAW, int nAH)
{
    // if(nAW < 0 || nAH < 0){ return; }
    DrawLine(nAX          , nAY          , nAX + nAW - 1, nAY          );
    DrawLine(nAX          , nAY          , nAX          , nAY + nAH - 1);
    DrawLine(nAX + nAW - 1, nAY          , nAX + nAW - 1, nAY + nAH - 1);
    DrawLine(nAX          , nAY + nAH - 1, nAX + nAW - 1, nAY + nAH - 1);
}

void MonitorDrawArea::DrawLoop(int nAX1, int nAY1, int nAX2, int nAY2, int nAX3, int nAY3)
{
    DrawLine(nAX1, nAY1, nAX2, nAY2);
    DrawLine(nAX2, nAY2, nAX3, nAY3);
    DrawLine(nAX3, nAY3, nAX1, nAY1);
}

void MonitorDrawArea::AttributeCoverOperation(
        int nMouseXOnMap, int nMouseYOnMap, int nSize, std::function<void(int, int, int)> fnOperation)
{
    if(nSize <= 0){ return; }

    int nMX = nMouseXOnMap / 48;
    int nMY = nMouseYOnMap / 32;

    for(int nX = nMX - (nSize / 2); nX < nMX + (nSize + 1) / 2; ++nX){
        for(int nY = nMY - (nSize / 2); nY < nMY + (nSize + 1) / 2; ++nY){
            for(int nIndex = 0; nIndex < 4; ++nIndex){
                extern EditorMap g_EditorMap;
                if(g_EditorMap.ValidC(nX, nY)){
                    fnOperation(nX, nY, nIndex);
                }
            }
        }
    }
}

void MonitorDrawArea::DrawLight()
{
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->ShowLightLayer()){ return; }

    if(!m_LightUC){
        uint32_t pData[32][48];
        for(int nY = 0; nY < 32; ++nY){
            for(int nX = 0; nX < 48; ++nX){
                pData[nY][nX] = 0X80FF0000;
            }
        }
        m_LightUC = Fl_RGB_Image((uchar *)pData, 48, 32, 4, 0).copy(48, 32);
    }

    auto fnDrawLight = [this](int nX, int nY){
        DrawImage(m_LightUC, nX * 48 - m_OffsetX, nY * 32 - m_OffsetY);
    };

    extern EditorMap g_EditorMap;
    g_EditorMap.DrawLight(
            m_OffsetX / 48 - 2, m_OffsetY / 32 - 2, w() / 48 + 4, h() / 32 + 4, fnDrawLight);
}

void MonitorDrawArea::CreateRC()
{
    // for circle with nR:  it's of width nR + nR + 1
    //
    //     +------- center pixel
    //     |
    //     V
    // o o o o o <--- most right pixel
    // ^
    // |
    // +--- most left pixel

    for(int nR = 0; nR < (int)m_RC.size(); ++nR){
        for(int nColorIndex = 0; nColorIndex < 256; ++nColorIndex){

            int nSize = 2 * nR + 1;
            uint32_t *pData = new uint32_t[nSize * nSize];

            for(int nY = 0; nY < nSize; nY++){
                for(int nX = 0; nX < nSize; nX++){
                    if(LDistance2(nX, nY, nR, nR) <= nR * nR){
                        pData[nY * nSize + nX] = g_Color[nColorIndex];
                    }
                }
            }

            m_RC[nR][nColorIndex] = Fl_RGB_Image(
                    (uchar *)(pData), nSize, nSize, 4, 0).copy(nSize, nSize);
            delete pData;
        }
    }
}

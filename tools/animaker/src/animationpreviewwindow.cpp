/*
 * =====================================================================================
 *
 *       Filename: animationpreviewwindow.cpp
 *        Created: 7/22/2015 3:16:57 AM
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

#include "pngf.hpp"
#include "filesys.hpp"
#include "wilanimationinfo.hpp"
#include "animationinfowindow.hpp"
#include "animationset.hpp"
#include "animationpreviewwindow.hpp"
#include "wilimagepackage.hpp"
#include "mainwindow.hpp"
#include <algorithm>

PreviewWindow::PreviewWindow(int nX, int nY, int nW, int nH)
    : Fl_Double_Window(nX, nY, nW, nH, nullptr)
{
    // Fl_Double_Window::set_modal();
}

int PreviewWindow::handle(int nEvent)
{
    int nRet = Fl_Double_Window::handle(nEvent);
    switch(nEvent){
        case FL_PUSH:
            if(Fl::event_clicks()){
				extern AnimationPreviewWindow *g_AnimationPreviewWindow;
				int nFileIndex      = g_AnimationPreviewWindow->FileIndex();
				int nAnimationIndex = g_AnimationPreviewWindow->AnimationIndex();
                extern AnimationSet g_AnimationSet;
                g_AnimationSet.ImportMir2Animation(nFileIndex, nAnimationIndex);
                {
                    static AnimationInfoWindow *pWin = nullptr;
                    delete pWin; pWin = new AnimationInfoWindow;
                    pWin->Set(12);
                    pWin->RedrawAll();
                    pWin->ShowAll();
                }
                hide();
            }
            break;
        default:
            break;
    }
    return nRet;
}

AnimationPreviewWindow::AnimationPreviewWindow(uint32_t nFileIndex, uint32_t nAnimationIndex)
	: m_CurrentFrameIndex(0)
    , m_FrameCount(0)
    , m_LastFrameIndex(0)
    , m_ImageMaxW(0)
    , m_ImageMaxH(0)
    , m_MaxW(0)
    , m_MaxH(0)
{
    std::memset(m_DSX, 0, 100 * sizeof(int));
    std::memset(m_DSY, 0, 100 * sizeof(int));
    std::memset(m_PX,  0, 100 * sizeof(int));
    std::memset(m_PY,  0, 100 * sizeof(int));
    for(int i = 0; i < 100; ++i){
        m_PNG[0][i] = nullptr;
        m_PNG[1][i] = nullptr;
    }
    // parameter nFileIndex, nAnimationIndex is for caching
    // otherwise we can just use nAnimationIndex
    extern ValidAnimationWindow *g_ValidAnimationWindow;
    if(false
            || nFileIndex      ==  0    // 1, 2, ..., 20 for Mon-1, ..., Mon-20
            || nFileIndex      >  20
            || nAnimationIndex >= (uint32_t)g_ValidAnimationWindow->AnimationCount()
      ){
        m_FileIndex      = 1;
        m_AnimationIndex = 0;
    }else{
        m_FileIndex      = nFileIndex;
        m_AnimationIndex = nAnimationIndex;
    }

    int       nMaxW    = 0;
    int       nMaxH    = 0;
    int       nDataLen = 0;
    uint32_t *pData    = nullptr;

	extern int g_MonsterWilFileStartIndex[];
	extern int g_MonsterShadowWilFileStartIndex[];

    int nBase0 = g_MonsterWilFileStartIndex[m_FileIndex - 1]       % 2;
    int nBase1 = g_MonsterShadowWilFileStartIndex[m_FileIndex - 1] % 2;
    int nBaseIndex  = WilAnimationStartBaseIndex(nFileIndex, nAnimationIndex, 0, 5);
    int nFrameCount = WilAnimationFrameCount(nFileIndex, nAnimationIndex, 0, 5);

    for(int nFrame = 0; nFrame < nFrameCount; ++nFrame){
        extern WilImagePackage g_WilImagePackage[2];
        if(true
                && g_WilImagePackage[0].SetIndex(nFrame + nBaseIndex + nBase0)
                && g_WilImagePackage[0].CurrentImageValid()
                && g_WilImagePackage[1].SetIndex(nFrame + nBaseIndex + nBase1)
                && g_WilImagePackage[1].CurrentImageValid()
          ){
            auto stInfo0 = g_WilImagePackage[0].CurrentImageInfo();
            auto stInfo1 = g_WilImagePackage[1].CurrentImageInfo();

            {// non-shadow layer
                char szSaveFileName[128];
                //0 12 2 3 3 5
                //1 2  3 4 5 6
                //1: shadow or not
                //2: file index 1 ~ 20: Mon-1 ~ Mon-20
                //3: monster index in file
                //4: status
                //5: direction
                //6: frame index
                std::sprintf(szSaveFileName, "./IMG/0%02d%02d005%02d.PNG", m_FileIndex, m_AnimationIndex, nFrame);
                if(!FileSys::FileExist(szSaveFileName)){
                    if(nDataLen < stInfo0.shWidth * stInfo0.shHeight){
                        delete pData;
                        pData    = new uint32_t[stInfo0.shWidth * stInfo0.shHeight];
                        nDataLen = stInfo0.shWidth * stInfo0.shHeight;
                    }
                    g_WilImagePackage[0].Decode(pData, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
                    pngf::saveRGBABuffer((uint8_t *)pData,
                            stInfo0.shWidth, stInfo0.shHeight,
                            szSaveFileName);
                }
                m_PNG[0][nFrame] = Fl_Shared_Image::get(szSaveFileName);
            }

            {// shadow layer
                char szSaveFileName[128];
                //0 12 2 3 3 5
                //1 2  3 4 5 6
                //1: shadow or not
                //2: file index 1 ~ 20: Mon-1 ~ Mon-20
                //3: monster index in file
                //4: status
                //5: direction
                //6: frame index
                std::sprintf(szSaveFileName, "./IMG/1%02d%02d005%02d.PNG", m_FileIndex, m_AnimationIndex, nFrame);
                if(!FileSys::FileExist(szSaveFileName)){
                    if(nDataLen < stInfo1.shWidth * stInfo1.shHeight){
                        delete pData;
                        pData    = new uint32_t[stInfo1.shWidth * stInfo1.shHeight];
                        nDataLen = stInfo1.shWidth * stInfo1.shHeight;
                    }
                    g_WilImagePackage[1].Decode(pData, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
                    pngf::saveRGBABuffer((uint8_t *)pData,
                            stInfo1.shWidth, stInfo1.shHeight,
                            szSaveFileName);
                }
                m_PNG[1][nFrame] = Fl_Shared_Image::get(szSaveFileName);
            }

            m_DSX[nFrame] = stInfo1.shPX - stInfo0.shPX;
            m_DSY[nFrame] = stInfo1.shPY - stInfo0.shPY;
            m_PX [nFrame] = stInfo0.shPX;
            m_PY [nFrame] = stInfo0.shPY;

            // m_PNG[0]: start: (x - dx, y - dy)
            // m_PNG[1]: start: (x     , y     )

            if(m_PNG[0][nFrame] == nullptr || m_PNG[1][nFrame] == nullptr){
                continue;
            }
            int nMinX = (std::min<int>)(0 - m_DSX[nFrame], 0);
            int nMinY = (std::min<int>)(0 - m_DSY[nFrame], 0);
            int nMaxX = (std::max<int>)(0 - m_DSX[nFrame] + m_PNG[0][nFrame]->w(), 0 + m_PNG[1][nFrame]->w());
            int nMaxY = (std::max<int>)(0 - m_DSY[nFrame] + m_PNG[0][nFrame]->h(), 0 + m_PNG[1][nFrame]->h());

            nMaxW = (std::max<int>)(nMaxW, nMaxX - nMinX);
            nMaxH = (std::max<int>)(nMaxH, nMaxY - nMinY);

            m_LastFrameIndex = nFrame;
            m_FrameCount++;
        }else{
            m_PNG[0][nFrame] = nullptr;
            m_PNG[1][nFrame] = nullptr;
        }
    }

    delete pData;

    m_MaxH      = (std::max<int16_t>)(((std::min<int64_t>)(((int16_t)(nMaxH * 1.5)), (int16_t)(nMaxH + 40))), (int16_t)250);
    m_MaxW      = (std::max<int16_t>)(((std::min<int64_t>)(((int16_t)(nMaxW * 1.5)), (int16_t)(nMaxW + 40))), (int16_t)250);
    m_ImageMaxW = nMaxW;
    m_ImageMaxH = nMaxH;

    m_Window = new PreviewWindow(0, 0, m_MaxW, m_MaxH);
    m_Window->labelfont(4);
    m_Window->end();

    if(m_FrameCount == 0){
        return;
    }

    Fl::remove_timeout(TimeoutCallback);
    Fl::add_timeout(0.2, TimeoutCallback, this);
}

void AnimationPreviewWindow::ShowAll()
{
    m_Window->show();
}

void AnimationPreviewWindow::RedrawAll()
{
    if(!m_Window->shown()){
        return;
    }

    m_Window->redraw();
    if(m_FrameCount == 0){
        return;
    }

    if(false
            || m_PNG[0][m_CurrentFrameIndex] == nullptr
            || m_PNG[0][m_CurrentFrameIndex] == nullptr
      ){
        return;
    }

    m_Window->make_current();
    Fl::check();

    int nMinX = (std::min<int>)(0 - m_DSX[m_CurrentFrameIndex], 0);
    int nMinY = (std::min<int>)(0 - m_DSY[m_CurrentFrameIndex], 0);
    int nMaxX = (std::max<int>)(0 - m_DSX[m_CurrentFrameIndex] + m_PNG[0][m_CurrentFrameIndex]->w(), 0 + m_PNG[1][m_CurrentFrameIndex]->w());
    int nMaxY = (std::max<int>)(0 - m_DSY[m_CurrentFrameIndex] + m_PNG[0][m_CurrentFrameIndex]->h(), 0 + m_PNG[1][m_CurrentFrameIndex]->h());

    // int nW = nMaxX - nMinX;
    // int nH = nMaxY - nMinY;

    int nMaxW = (std::max<int>)(nMaxW, nMaxX - nMinX);
    int nMaxH = (std::max<int>)(nMaxH, nMaxY - nMinY);

    int nStartX = (m_MaxW - m_ImageMaxW) / 2;
    int nStartY = (m_MaxH - m_ImageMaxH) / 2;

    int nDX = m_PX[m_CurrentFrameIndex] - m_PX[0];
    int nDY = m_PY[m_CurrentFrameIndex] - m_PY[0];

    // int nDX = m_PX[m_CurrentFrameIndex] - m_PX[m_LastFrameIndex];
    // int nDY = m_PY[m_CurrentFrameIndex] - m_PY[m_LastFrameIndex];

    m_PNG[1][m_CurrentFrameIndex]->draw(
            nDX + m_DSX[m_CurrentFrameIndex] + nStartX, 
            nDY + m_DSY[m_CurrentFrameIndex] + nStartY);
    m_PNG[0][m_CurrentFrameIndex]->draw(nDX + nStartX, nDY + nStartY);
}

void AnimationPreviewWindow::UpdateFrame()
{
    if (m_FrameCount > 0){
        do{
            m_CurrentFrameIndex++;
            m_CurrentFrameIndex %= 10;
        }while(!(m_PNG[0][m_CurrentFrameIndex] && m_PNG[1][m_CurrentFrameIndex]));
    }
}

void AnimationPreviewWindow::TimeoutCallback(void *p)
{
    if(p){
        ((AnimationPreviewWindow*)p)->UpdateFrame();
        ((AnimationPreviewWindow*)p)->RedrawAll();
        Fl::repeat_timeout(0.2, TimeoutCallback, p);
    }else{
        Fl::remove_timeout(TimeoutCallback);
    }
}

void AnimationPreviewWindow::EventCallback(void *p)
{
    Fl::remove_timeout(AnimationPreviewWindow::TimeoutCallback);
    ((AnimationPreviewWindow *)p)->HideAll();
}

AnimationPreviewWindow::~AnimationPreviewWindow()
{
    Fl::remove_timeout(TimeoutCallback);
    delete m_Window; m_Window = nullptr;
}

void AnimationPreviewWindow::HideAll()
{
    // TODO: bug here
    m_Window->hide();
}

int AnimationPreviewWindow::FileIndex()
{
    return m_FileIndex;
}

int AnimationPreviewWindow::AnimationIndex()
{
    return m_AnimationIndex;
}

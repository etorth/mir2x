/*
 * =====================================================================================
 *
 *       Filename: animationpreviewwindow.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 06/23/2016 00:59:46
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

#include <FL/Fl.H>
#include "animationdb.hpp"
#include "animationdraw.hpp"
#include "animationpreviewwindow.hpp"

AnimationPreviewWindow::PreviewWindow::PreviewWindow(int nX, int nY, int nW, int nH)
    : Fl_Double_Window(nX, nY, nW, nH, nullptr)
{
    // Fl_Double_Window::set_modal();
}

AnimationPreviewWindow::PreviewWindow::~PreviewWindow()
{
    extern AnimationPreviewWindow *g_AnimationPreviewWindow;
    g_AnimationPreviewWindow->ResetMonsterID(0);
}

int AnimationPreviewWindow::PreviewWindow::handle(int nEvent)
{
    int nRet = Fl_Double_Window::handle(nEvent);
    switch(nEvent){
        case FL_PUSH:
            if(Fl::event_clicks()){
                // 1. set the selected monster id
                extern AnimationDraw g_AnimationDraw;
                extern AnimationPreviewWindow *g_AnimationPreviewWindow;
                g_AnimationDraw.MonsterID = g_AnimationPreviewWindow->MonsterID();

                // 2. disable the timeout function to prevent it update the g_AnimationDB
                g_AnimationPreviewWindow->ResetMonsterID(0);

                // 3. hide current preview window
                hide();
            }
            break;
        default:
            break;
    }
    return nRet;
}

AnimationPreviewWindow::AnimationPreviewWindow()
    : m_Window(nullptr)
    , m_MonsterID(0)
{}

AnimationPreviewWindow::~AnimationPreviewWindow()
{
    ResetMonsterID(0);
    delete m_Window; m_Window = nullptr;
}

void AnimationPreviewWindow::ShowAll()
{
    if(m_Window){ m_Window->show(); }
}

void AnimationPreviewWindow::RedrawAll()
{
    if(!m_Window){ return; }
    if(!m_Window->shown()){ return; }

    m_Window->redraw();

    if(m_MonsterID){
        extern AnimationDB g_AnimationDB;
        auto & rstRecord = g_AnimationDB.RetrieveAnimation(m_MonsterID);

        if(rstRecord.Valid()){
            int nW = m_Window->w();
            int nH = m_Window->h();

            m_Window->make_current();
            Fl::check();

            rstRecord.Draw(nW / 2 + 20, nH - 20);
        }
    }
}

void AnimationPreviewWindow::UpdateFrame()
{
    if(!m_Window){ return; }
    if(!m_Window->shown()){ return; }

    if(m_MonsterID){
        extern AnimationDB g_AnimationDB;
        auto & rstRecord = g_AnimationDB.RetrieveAnimation(m_MonsterID);

        if(rstRecord.Valid()){
            rstRecord.Update();
        }
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

void AnimationPreviewWindow::HideAll()
{
    Fl::remove_timeout(TimeoutCallback);
    m_Window->hide();
}

void AnimationPreviewWindow::ResetMonsterID(uint32_t nMonsterID)
{
    // 0. we could stop here
    if(nMonsterID == m_MonsterID){ return; }

    // 1. remove the timeout function
    Fl::remove_timeout(TimeoutCallback);

    // 2. delete the window since each monster has different size
    //    and the window is created by its size
    delete m_Window; m_Window = nullptr;

    // 3. if empty monster id we stop here, so we can use this function
    //    to disable all functionality of AnimationPreviewWindow
    if(!m_MonsterID){ return; }

    extern AnimationDB g_AnimationDB;
    auto &rstRecord = g_AnimationDB.RetrieveAnimation(m_MonsterID);

    if(!rstRecord.Valid()){ return; }

    int nAnimationW = rstRecord.AnimationW(0, 0);
    int nAnimationH = rstRecord.AnimationH(0, 0);

    m_Window = new PreviewWindow(0, 0, nAnimationW + 20 * 2, nAnimationH + 20 * 2);
    m_Window->labelfont(4);
    m_Window->end();

    // 5. install the new timeout function
    Fl::add_timeout(0.2, TimeoutCallback, this);
}

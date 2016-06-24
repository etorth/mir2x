/*
 * =====================================================================================
 *
 *       Filename: animationpreviewwindow.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 06/23/2016 22:14:48
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
#include <Fl/fl_draw.H>

#include <algorithm>

#include "animationdb.hpp"
#include "animationdraw.hpp"
#include "animationpreviewwindow.hpp"

AnimationPreviewWindow::PreviewWindow::PreviewWindow(int nX, int nY, int nW, int nH)
    : Fl_Double_Window(nX, nY, nW, nH, nullptr)
{
    // Fl_Double_Window::set_modal();
}

AnimationPreviewWindow::PreviewWindow::~PreviewWindow()
{}

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

void AnimationPreviewWindow::PreviewWindow::draw()
{
    Fl_Double_Window::draw();
    extern AnimationPreviewWindow *g_AnimationPreviewWindow;
    uint32_t nMonsterID = g_AnimationPreviewWindow->MonsterID();

    if(nMonsterID){
        extern AnimationDB g_AnimationDB;
        auto & rstRecord = g_AnimationDB.RetrieveAnimation(nMonsterID);

        if(rstRecord.Valid()){
            int nW = w();
            int nH = h();

            // TODO: I have already get the of make_current()
            //       if we are drawing inside the draw() of the window, never call
            //       this function, if out side the class, we need to call it
            //
            //       if(this != m_Window){
            //          m_Window->make_current();
            //       }
            // make_current();
            Fl::check();
            rstRecord.Draw(nW / 2, nH - 100);

            return;
        }
    }

    // else there is nothing to draw, draw a "X"
    auto nColor = fl_color();
    fl_color(FL_RED);

    fl_line(0, 0, w(), h());
    fl_line(w(), 0, 0, h());

    fl_color(nColor);
}

AnimationPreviewWindow::AnimationPreviewWindow()
    : m_Window(nullptr)
    , m_MonsterID(0)
{
    m_Window = new PreviewWindow(0, 0, 0 + 100 * 2, 0 + 100 * 2);
    m_Window->labelfont(4);
    m_Window->set_modal();
    m_Window->end();
}

AnimationPreviewWindow::~AnimationPreviewWindow()
{
    Fl::remove_timeout(TimeoutCallback);
    delete m_Window; m_Window = nullptr;
}

void AnimationPreviewWindow::ShowAll()
{
    if(!m_Window){ return; }
    m_Window->show();
}

void AnimationPreviewWindow::RedrawAll()
{
    if(!m_Window){ return; }
    m_Window->redraw();
}

void AnimationPreviewWindow::UpdateFrame()
{
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
        uint32_t nMonsterID = ((AnimationPreviewWindow*)p)->MonsterID();
        if(nMonsterID){
            ((AnimationPreviewWindow*)p)->UpdateFrame();
            ((AnimationPreviewWindow*)p)->RedrawAll();
            Fl::repeat_timeout(0.2, TimeoutCallback, p);
        }else{
            ((AnimationPreviewWindow*)p)->HideAll();
        }
    }else{
        Fl::remove_timeout(TimeoutCallback);
    }
}

void AnimationPreviewWindow::HideAll()
{
    if(!m_Window){ return; }
    m_Window->hide();
}

void AnimationPreviewWindow::ResetMonsterID(uint32_t nMonsterID)
{
    // 0. we could stop here
    if(nMonsterID == m_MonsterID){ return; }

    // 1. remove the timeout function
    Fl::remove_timeout(TimeoutCallback);

    // 2. clear the monster id
    m_MonsterID = 0;

    // 3. prepare for change the size of preview window, previously I delete this
    //    window but it cause many problems, so I decide to use resize()
    int nW = 0;
    int nH = 0;

    // 4. if empty monster id we stop here, so we can use this function
    //    to disable all functionality of AnimationPreviewWindow
    if(nMonsterID){
        extern AnimationDB g_AnimationDB;
        auto &rstRecord = g_AnimationDB.RetrieveAnimation(nMonsterID);

        if(rstRecord.Valid()){
            int nAnimationW = rstRecord.AnimationW(0, 5);
            int nAnimationH = rstRecord.AnimationH(0, 5);

            if(nAnimationW > 0 && nAnimationH > 0){
                m_MonsterID = nMonsterID;

                nW = nAnimationW;
                nH = nAnimationH;

                rstRecord.ResetAction(0);
                rstRecord.ResetDirection(5);
            }
        }
    }

    if(m_Window){
        m_Window->resize( m_Window->x(), m_Window->y(),
                std::max<int>(m_Window->w(), nW + 100 * 2),
                std::max<int>(m_Window->h(), nH + 100 * 2));
    }
    
    if(m_MonsterID){
        ShowAll();
    }else{
        // TODO: even this is an empty monster, we don't remove the callback
        //       function since which causes more bugs
        HideAll();
    }

    // 5. install the new timeout function
    Fl::add_timeout(0.2, TimeoutCallback, this);
}

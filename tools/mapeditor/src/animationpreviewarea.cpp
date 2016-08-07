/*
 * =====================================================================================
 *
 *       Filename: animationpreviewarea.cpp
 *        Created: 06/28/2016 23:29:25
 *  Last Modified: 08/07/2016 10:59:21
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

#include "mainwindow.hpp"
#include "animationdb.hpp"
#include "animationdraw.hpp"
#include "animationpreviewarea.hpp"
#include "animationselectwindow.hpp"

AnimationPreviewArea::AnimationPreviewArea(int nX, int nY, int nW, int nH)
    : Fl_Box(nX, nY, nW, nH)
{}

AnimationPreviewArea::~AnimationPreviewArea()
{}

int AnimationPreviewArea::handle(int nEvent)
{
    int nRet = Fl_Box::handle(nEvent);
    switch(nEvent){
        case FL_PUSH:
            if(Fl::event_clicks()){
                extern AnimationDraw g_AnimationDraw;
                extern AnimationSelectWindow *g_AnimationSelectWindow;
                uint32_t nMonsterID = g_AnimationSelectWindow->MonsterID();
                int nAction0 = g_AnimationSelectWindow->Action();
                int nDirection0 = g_AnimationSelectWindow->Direction();

                // we have a new setting, use it, otherwise just keep the old one
                if(nMonsterID && nAction0 >= 0 && nDirection0 >= 0){
                    g_AnimationDraw.MonsterID = nMonsterID;
                    g_AnimationDraw.Action    = (uint32_t)(nAction0);
                    g_AnimationDraw.Direction = (uint32_t)(nDirection0);
                }

                // 1. remove animationselectwindow's callback
                Fl::remove_timeout(AnimationSelectWindow::TimeoutCallback);

                // 2. add new timeout for mainwindow draw
                Fl::remove_timeout(MainWindow::UpdateAnimationFrame);
                Fl::add_timeout(0.2, MainWindow::UpdateAnimationFrame, nullptr);

                // 3. hide all windows for animation selection
                extern AnimationSelectWindow *g_AnimationSelectWindow;
                g_AnimationSelectWindow->HideAll();
            }
            break;
        default:
            break;
    }
    return nRet;
}

void AnimationPreviewArea::draw()
{
    Fl_Box::draw();
    extern AnimationSelectWindow *g_AnimationSelectWindow;
    int nFrame0 = g_AnimationSelectWindow->Frame();
    int nAction0 = g_AnimationSelectWindow->Action();
    int nDirection0 = g_AnimationSelectWindow->Direction();
    uint32_t nMonsterID = g_AnimationSelectWindow->MonsterID();

    if(nMonsterID == 0 || nAction0 < 0 || nDirection0 < 0 || nFrame0 < 0){ return; }

    uint32_t nFrame = (uint32_t)(nFrame0);
    uint32_t nAction = (uint32_t)(nAction0);
    uint32_t nDirection = (uint32_t)(nDirection0);

    extern AnimationDB g_AnimationDB;
    auto & rstRecord = g_AnimationDB.RetrieveAnimation(nMonsterID);

    if(!rstRecord.Valid()){ return; }
    if(!rstRecord.ResetFrame(nAction, nDirection, nFrame)){ return; }

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
    rstRecord.Draw(x() + nW / 2, y() + nH - 100);
}

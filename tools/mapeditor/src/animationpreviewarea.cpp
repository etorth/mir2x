/*
 * =====================================================================================
 *
 *       Filename: animationpreviewarea.cpp
 *        Created: 06/28/2016 23:29:25
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
                extern AnimationDraw g_animationDraw;
                extern AnimationSelectWindow *g_animationSelectWindow;
                uint32_t nMonsterID = g_animationSelectWindow->MonsterID();
                int nAction0 = g_animationSelectWindow->Action();
                int nDirection0 = g_animationSelectWindow->Direction();

                // we have a new setting, use it, otherwise just keep the old one
                if(nMonsterID && nAction0 >= 0 && nDirection0 >= 0){
                    g_animationDraw.MonsterID = nMonsterID;
                    g_animationDraw.Action    = to_u32(nAction0);
                    g_animationDraw.Direction = to_u32(nDirection0);
                }

                // 1. remove animationselectwindow's callback
                Fl::remove_timeout(AnimationSelectWindow::TimeoutCallback);

                // 2. add new timeout for mainwindow draw
                Fl::remove_timeout(MainWindow::UpdateAnimationFrame);
                Fl::add_timeout(0.2, MainWindow::UpdateAnimationFrame, nullptr);

                // 3. hide all windows for animation selection
                extern AnimationSelectWindow *g_animationSelectWindow;
                g_animationSelectWindow->HideAll();
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
    extern AnimationSelectWindow *g_animationSelectWindow;
    int nFrame0 = g_animationSelectWindow->Frame();
    int nAction0 = g_animationSelectWindow->Action();
    int nDirection0 = g_animationSelectWindow->Direction();
    uint32_t nMonsterID = g_animationSelectWindow->MonsterID();

    if(nMonsterID == 0 || nAction0 < 0 || nDirection0 < 0 || nFrame0 < 0){ return; }

    uint32_t nFrame = to_u32(nFrame0);
    uint32_t nAction = to_u32(nAction0);
    uint32_t nDirection = to_u32(nDirection0);

    extern AnimationDB g_animationDB;
    auto & rstRecord = g_animationDB.RetrieveAnimation(nMonsterID);

    if(!rstRecord.Valid()){ return; }
    if(!rstRecord.ResetFrame(nAction, nDirection, nFrame)){ return; }

    int nW = w();
    int nH = h();

    // TODO: I have already understand how to use make_current()
    //       if we are drawing inside the draw() of the window, never call
    //       this function, if out side the class, we need to call it
    //
    //       if(this != m_window){
    //          m_window->make_current();
    //       }
    // make_current();
    Fl::check();

    extern AnimationSelectWindow *g_animationSelectWindow;
    int nCenterX = nW / 2;
    int nCenterY = nH - 100;
    int nCenterR = g_animationSelectWindow->R(); 

    auto stOldColor = fl_color();
    fl_color(FL_YELLOW);
    fl_pie(x() + nCenterX - nCenterR, y() + nCenterY - nCenterR, 2 * nCenterR, 2 * nCenterR, 0.0, 360.0);
    fl_color(FL_BLUE);
    fl_circle(x() + nCenterX * 1.0, y() + nCenterY * 1.0, nCenterR * 1.0);
    {
        char szRInfo[64];
        std::sprintf(szRInfo, "R: %d", nCenterR);
        fl_color(FL_RED);
        fl_draw(szRInfo, x() + 15, y() + nH - 15);
    }
    fl_color(stOldColor);

    rstRecord.Draw(x() + nCenterX, y() + nCenterY);
}

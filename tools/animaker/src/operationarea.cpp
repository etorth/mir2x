/*
 * =====================================================================================
 *
 *       Filename: operationarea.cpp
 *        Created: 09/03/2015 03:48:41 AM
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
#include <FL/fl_draw.H>

#include "mainwindow.hpp"
#include "animationset.hpp"
#include "operationarea.hpp"

OperationArea::OperationArea(int x, int y, int w, int h, const char *label)
    : Fl_Box(x, y, w, h, label)
    , m_CenterPositionX(0)
    , m_CenterPositionY(0)
{
    m_CenterPositionX = Fl_Box::w() / 2;
    m_CenterPositionY = Fl_Box::h() / 2;
}

OperationArea::OperationArea(int x, int y, int w, int h)
    : OperationArea(x, y, w, h, nullptr)
{}

OperationArea::~OperationArea()
{}

void OperationArea::draw()
{
    Fl_Box::draw();
    DrawAnimation();
}

void OperationArea::DrawAnimation()
{
    extern MainWindow   *g_MainWindow;
    extern AnimationSet  g_AnimationSet;

    if(g_AnimationSet.Valid()){
        extern int g_AnimationSetWinCenterX;
        extern int g_AnimationSetWinCenterY;
        g_AnimationSetWinCenterX = x() + m_CenterPositionX;
        g_AnimationSetWinCenterY = y() + m_CenterPositionY;

        g_AnimationSet.Draw(g_AnimationSetWinCenterX, g_AnimationSetWinCenterY);
    }

    g_MainWindow->RedrawAll();
}

int OperationArea::handle(int nEvent)
{
    static int nOldMouseX = 0;
    static int nOldMouseY = 0;

    int nMouseX = Fl::event_x();
    int nMouseY = Fl::event_y();

    int nRet = Fl_Box::handle(nEvent);
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->TestMode() && !g_MainWindow->TestAnimation()){
        return nRet;
    }

    switch(nEvent){
        case FL_DRAG:
            {
                m_CenterPositionX += (nMouseX - nOldMouseX);
                m_CenterPositionY += (nMouseY - nOldMouseY);

                if(m_CenterPositionX < 0){ m_CenterPositionX = 0; }
                if(m_CenterPositionY < 0){ m_CenterPositionY = 0; }

                if(m_CenterPositionX >= w()){ m_CenterPositionX = w(); }
                if(m_CenterPositionY >= h()){ m_CenterPositionY = h(); }

                extern MainWindow *g_MainWindow;
                g_MainWindow->RedrawAll();
                break;
            }
        case FL_RELEASE:
            {
                fl_cursor(FL_CURSOR_DEFAULT);
                break;
            }
        case FL_PUSH:
            {
                extern MainWindow *g_MainWindow;
                if(g_MainWindow->EnableEdit()){
                    if(g_MainWindow->EditCover()){ fl_cursor(FL_CURSOR_MOVE); }
                }
                nRet = 1;
                break;
            }
        default:
            break;
    }

    {
        // save last cursor position
        nOldMouseX = nMouseX;
        nOldMouseY = nMouseY;
    }
    return nRet;
}

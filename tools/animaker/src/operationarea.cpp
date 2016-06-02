#include "operationarea.hpp"
#include "animationset.hpp"
#include "mainwindow.hpp"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

OperationArea::OperationArea(int x, int y, int w, int h, const char *label)
    : Fl_Box(x, y, w, h, label)
{}

OperationArea::OperationArea(int x, int y, int w, int h)
    : Fl_Box(x, y, w, h, nullptr)
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
    extern AnimationSet  g_AnimationSet;
	extern MainWindow   *g_MainWindow;
    extern int           g_AnimationSetPositionX;
    extern int           g_AnimationSetPositionY;
    // g_MainWindow->MakeCurrent();
    g_AnimationSet.Draw(x() + g_AnimationSetPositionX, y() + g_AnimationSetPositionY);
	g_MainWindow->RedrawAll();
}

int OperationArea::handle(int nEvent)
{
    static int nOldMouseX = 0;
    static int nOldMouseY = 0;
    int        nMouseX    = Fl::event_x();
    int        nMouseY    = Fl::event_y();

    int nRet = Fl_Box::handle(nEvent);
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->TestMode() && !g_MainWindow->TestAnimation()){
        return nRet;
    }

    switch(nEvent){
        case FL_DRAG:
            {
                extern int g_AnimationSetPositionX;
                extern int g_AnimationSetPositionY;
                g_AnimationSetPositionX += (nMouseX - nOldMouseX);
                g_AnimationSetPositionY += (nMouseY - nOldMouseY);
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
                // printf("%3d %3d\n", nMouseX - x(), nMouseY - y());
                extern MainWindow *g_MainWindow;
                if(g_MainWindow->EnableEdit()){
                    extern int          g_AnimationSetPositionX;
                    extern int          g_AnimationSetPositionY;
					extern AnimationSet g_AnimationSet;
                    if(true
                            && g_MainWindow->EditCover()
                            && g_AnimationSet.InCover(
                                nMouseX - x() - g_AnimationSetPositionX,
                                nMouseY - y() - g_AnimationSetPositionY)){
                        fl_cursor(FL_CURSOR_MOVE);
                    }
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

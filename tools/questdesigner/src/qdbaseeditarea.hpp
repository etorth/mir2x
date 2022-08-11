#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>

class QD_BaseEditArea: public Fl_Scroll
{
    protected:
        QD_BaseEditArea(int argX, int argY, int argW, int argH, const char *argLabel)
            : Fl_Scroll(argX, argY, argW, argH, argLabel)
        {
            box(FL_DOWN_BOX);
        }
};

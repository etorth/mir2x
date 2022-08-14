#pragma once
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>

class QD_CondChecker_base: public Fl_Group
{
    public:
        QD_CondChecker_base(int argX, int argY, int argW, int argH, const char *argL = 0)
            : Fl_Group(argX, argY, argW, argH, argL)
        {}

    protected:
        void draw() override
        {
            Fl_Group::draw();
            fl_draw_box(FL_UP_FRAME, x(), y(), w(), h(), color());
        }
};

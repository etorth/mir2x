#include "qddesktop.hpp"

QD_Desktop::QD_Desktop(int argX, int argY, int argW, int argH, const char *argLabel)
    : Fl_Scroll(argX, argY, argW, argH, argLabel)
{}

void QD_Desktop::draw()
{
    Fl_Scroll::draw();
    fl_rectf(x(), y(), w(), h(), 0x567b9100); // skyblue
    fl_draw_box(FL_DOWN_FRAME, x(), y(), w(), h(), color());
}

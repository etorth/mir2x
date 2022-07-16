#include "qddesktop.hpp"

QD_Desktop::QD_Desktop(int x, int y, int w, int h, const char *l)
    : Fl_Scroll(x, y, w, h, l)
{}

void QD_Desktop::draw()
{
    Fl_Scroll::draw();
    fl_color(0x567b9100); // skyblue
    fl_rectf(x(), y(), w(), h());
}

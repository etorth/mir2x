#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>

class QD_Desktop: public Fl_Scroll
{
    public:
        QD_Desktop(int, int, int, int, const char * = nullptr);

    protected:
        void draw() override;
};

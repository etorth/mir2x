#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Box.H>


class OperationArea: public Fl_Box
{
    public:
        OperationArea(int, int, int, int);
        OperationArea(int, int, int, int, const char *);
       ~OperationArea();
    public:
       void draw();
       int  handle(int);
    private:
       void DrawAnimation();
};

#pragma once
#include <atomic>
#include <memory>
#include <cstdint>
#include <FL/Fl_Button.H>
#include <FL/Fl_Shared_Image.H>
#include "fflerror.hpp"

class QD_IconButton: public Fl_Button
{
    public:
        QD_IconButton(int argX, int argY, int argW, int argH, const char *argLabel = 0)
            : Fl_Button(argX, argY, argW, argH, argLabel)
        {}

    public:
        void image(Fl_Image *img)
        {
            // fluid calls o->image(img)
            // but be aware that Fl_Widget::image(img) is not a virtual function

            Fl_Button::image(img);
            if(Fl_Button::image()){
                Fl_Button::image()->scale(w(), h(), 0, 1);
            }
        }

    public:
        void resize(int argX, int argY, int argW, int argH) override
        {
            Fl_Button::resize(argX, argY, argW, argH);
            if(Fl_Button::image()){
                Fl_Button::image()->scale(w(), h(), 0, 1);
            }
        }

    public:
        void draw() override
        {
            Fl_Button::draw();
            if(Fl_Button::image()){
                Fl_Button::image()->draw(x(), y(), w(), h(), 0, 0);
            }
        }
};

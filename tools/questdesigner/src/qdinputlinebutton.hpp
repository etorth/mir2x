#pragma once
#include <vector>
#include <FL/Fl.H>
#include <Fl/Fl_Group.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Button.H>
#include "qdbaseinputbutton.hpp"

class QD_InputLineButton: public QD_BaseInputButton
{
    public:
        QD_InputLineButton(int argX, int argY, int argW, int argH, const char *argLabel = nullptr)
            : QD_BaseInputButton(argX, argY, argW, argH, argLabel)
        {
            {
                m_input  = new Fl_Input (argX, argY, argW, argH);
                m_button = new Fl_Button(argX, argY, argW, argH, argLabel);

                edit(false);
                m_button->callback(+[](Fl_Widget *, void *p)
                {
                    static_cast<QD_InputLineButton *>(p)->edit(true);
                    static_cast<QD_InputLineButton *>(p)->take_focus();
                }, this);
            }
            this->end();
        }
};

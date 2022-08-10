#pragma once
#include <vector>
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Button.H>
#include "qdbaseinputbutton.hpp"

class QD_InputMultilineButton: public QD_BaseInputButton
{
    public:
        QD_InputMultilineButton(int argX, int argY, int argW, int argH, const char *argLabel = nullptr)
            : QD_BaseInputButton(argX, argY, argW, argH, argLabel)
        {
            {
                m_input  = new Fl_Multiline_Input(argX, argY, argW, argH);
                m_button = new Fl_Button         (argX, argY, argW, argH, argLabel);

                edit(false);
                m_button->callback(+[](Fl_Widget *, void *p)
                {
                    static_cast<QD_BaseInputButton *>(p)->edit(true);
                    static_cast<QD_BaseInputButton *>(p)->take_focus();
                }, this);
            }
            this->end();
        }
};

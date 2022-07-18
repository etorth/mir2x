#include "totype.hpp"
#include "dbcomid.hpp"
#include "qdinputlinebutton.hpp"

QD_InputLineButton::QD_InputLineButton(int argX, int argY, int argW, int argH, const char *argLabel)
    : Fl_Group(argX, argY, argW, argH)
{
    {
        m_input  = new Fl_Input (argX, argY, argW, argH);
        m_button = new Fl_Button(argX, argY, argW, argH, argLabel);

        m_button->callback(+[](Fl_Widget *, void *p)
        {
            static_cast<QD_InputLineButton *>(p)->m_input ->show();
            static_cast<QD_InputLineButton *>(p)->m_button->hide();
        }, this);
    }
    this->end();
}

int QD_InputLineButton::handle(int event)
{
    printf("event: %d\n", event);
    switch(event){
        case FL_UNFOCUS:
            {
                m_input ->hide();
                m_button->show();
                return 1;
            }
        default:
            {
                return Fl_Group::handle(event);
            }
    }
}

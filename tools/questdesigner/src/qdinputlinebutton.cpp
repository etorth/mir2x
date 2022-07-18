#include "totype.hpp"
#include "dbcomid.hpp"
#include "qdinputlinebutton.hpp"

QD_InputLineButton::QD_InputLineButton(int argX, int argY, int argW, int argH, const char *argLabel)
    : Fl_Group(argX, argY, argW, argH)
{
    {
        m_input  = new Fl_Input (argX, argY, argW, argH);
        m_button = new Fl_Button(argX, argY, argW, argH, argLabel);

        edit(false);
        m_button->callback(+[](Fl_Widget *, void *p)
        {
            static_cast<QD_InputLineButton *>(p)->edit(true);
        }, this);
    }
    this->end();
}

void QD_InputLineButton::edit(bool enable)
{
    if(enable){
        m_input ->show();
        m_button->hide();
    }
    else{
        m_input ->hide();
        m_button->show();
    }
}

int QD_InputLineButton::handle(int event)
{
    switch(event){
        case FL_UNFOCUS:
            {
                edit(false);
                return 1;
            }
        default:
            {
                return Fl_Group::handle(event);
            }
    }
}

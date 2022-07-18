#include "strf.hpp"
#include "totype.hpp"
#include "dbcomid.hpp"
#include "qdinputlinebutton.hpp"

QD_BaseInputButton::QD_BaseInputButton(int argX, int argY, int argW, int argH, const char *argLabel)
    : Fl_Group(argX, argY, argW, argH)
    , m_defaultLabel(argLabel ? argLabel : "默认标签")
{}

void QD_BaseInputButton::edit(bool enable)
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

void QD_BaseInputButton::wrap(bool b)
{
    m_input->wrap(b);
}

int QD_BaseInputButton::handle(int event)
{
    const auto result = Fl_Group::handle(event);
    if(event == FL_UNFOCUS){
        edit(false);
    }

    if(const auto textptr = m_input->value(); str_haschar(textptr)){
        m_button->copy_label(textptr);
    }
    else{
        m_button->copy_label(m_defaultLabel.c_str());
    }
    return result;
}

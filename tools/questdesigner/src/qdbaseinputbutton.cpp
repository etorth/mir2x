#include <sstream>
#include <FL/fl_draw.H>
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

void QD_BaseInputButton::input_align(Fl_Align align)
{
    m_inputAlign = align;
}

int QD_BaseInputButton::handle(int event)
{
    const auto result = Fl_Group::handle(event);
    if(event == FL_UNFOCUS){
        edit(false);
    }

    if(const auto textptr = m_input->value(); str_haschar(textptr)){
        std::stringstream ss(textptr);
        std::string token;
        std::string currTitle;

        while(std::getline(ss, token, '\n')){
            if(currTitle.empty()){
                currTitle = token;
            }
            else{
                currTitle += "\n";
                currTitle += token;
            }

            int titleW = 0;
            int titleH = 0;
            fl_measure(currTitle.c_str(), titleW, titleH, false);

            if(titleH >= m_button->h()){
                break;
            }

            m_button->copy_label(currTitle.c_str());
            m_button->align(Fl_Align(FL_ALIGN_CLIP | FL_ALIGN_INSIDE | m_inputAlign));
        }
    }
    else{
        m_button->copy_label(m_defaultLabel.c_str());
        m_button->align(Fl_Align(FL_ALIGN_CLIP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER));
    }
    return result;
}

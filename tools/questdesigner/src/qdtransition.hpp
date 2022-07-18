#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include "qdinputmultilinebutton.hpp"

class QD_Transition: public Fl_Group
{
    private:
        Fl_Box *m_enterBox = nullptr;

    private:
        QD_InputMultilineButton *m_callback = nullptr;

    public:
        QD_Transition(int, int, int, int, const char * = nullptr);
};

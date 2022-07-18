#pragma once
#include <FL/Fl.H>
#include <Fl/Fl_Group.H>
#include <Fl/Fl_Multiline_Input.H>
#include "qdinputlinebutton.hpp"
#include "qdinputmultilinebutton.hpp"

class QD_NodeEditGroup: public Fl_Group
{
    private:
        QD_InputLineButton *m_title = nullptr;
        QD_InputMultilineButton *m_questLog = nullptr;

    private:
        QD_InputMultilineButton *m_enterTrigger = nullptr;

    public:
        QD_NodeEditGroup(int, int, int, int, const char * = nullptr);

    public:
        int handle(int) override;
};

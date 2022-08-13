#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Multiline_Input.H>
#include "qdbaseeditarea.hpp"
#include "qdinputlinebutton.hpp"
#include "qdinputmultilinebutton.hpp"

class QD_NodeEditArea: public QD_BaseEditArea
{
    private:
        QD_InputLineButton *m_title = nullptr;
        QD_InputMultilineButton *m_questLog = nullptr;

    private:
        QD_InputMultilineButton *m_enterTrigger = nullptr;

    private:
        QD_InputMultilineButton *m_leaveTrigger = nullptr;

    public:
        QD_NodeEditArea(int, int, int, int, const char * = nullptr);

    public:
        int handle(int) override;
};

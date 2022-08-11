#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Multiline_Input.H>
#include "qdinputlinebutton.hpp"
#include "qdinputmultilinebutton.hpp"

class QD_NodeEditArea: public Fl_Scroll
{
    private:
        QD_InputLineButton *m_title = nullptr;
        QD_InputMultilineButton *m_questLog = nullptr;

    private:
        QD_InputMultilineButton *m_enterTrigger = nullptr;

    public:
        QD_NodeEditArea(int, int, int, int, const char * = nullptr);

    public:
        void draw();
        int handle(int) override;
};

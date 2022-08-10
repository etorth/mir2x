#pragma once
#include <string>
#include <vector>
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>

class QD_BaseInputButton: public Fl_Group
{
    protected:
        Fl_Input  *m_input  = nullptr;
        Fl_Button *m_button = nullptr;

    protected:
        const std::string m_defaultLabel;

    protected:
        Fl_Align m_inputAlign = FL_ALIGN_CENTER;

    protected:
        QD_BaseInputButton(int, int, int, int, const char * = nullptr);

    public:
        void edit(bool);

    public:
        void wrap(bool b);

    public:
        void input_align(Fl_Align);

    protected:
        int handle(int);

    protected:
        void create_widget(bool);
};

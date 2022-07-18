#pragma once
#include <string>
#include <vector>
#include <FL/Fl.H>
#include <Fl/Fl_Group.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Button.H>

class QD_BaseInputButton: public Fl_Group
{
    protected:
        Fl_Input  *m_input  = nullptr;
        Fl_Button *m_button = nullptr;

    protected:
        const std::string m_defaultLabel;

    protected:
        QD_BaseInputButton(int, int, int, int, const char * = nullptr);

    public:
        void edit(bool);

    public:
        void wrap(bool b);

    protected:
        int handle(int);
};

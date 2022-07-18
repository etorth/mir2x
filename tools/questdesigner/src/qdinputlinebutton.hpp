#pragma once
#include <vector>
#include <FL/Fl.H>
#include <Fl/Fl_Group.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Button.H>

class QD_InputLineButton: public Fl_Group
{
    private:
        Fl_Input  *m_input  = nullptr;
        Fl_Button *m_button = nullptr;

    public:
        QD_InputLineButton(int, int, int, int, const char * = nullptr);

    public:
        void edit(bool);

    protected:
        int handle(int);
};

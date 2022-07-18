#pragma once
#include <FL/Fl.H>
#include <Fl/Fl_Group.H>
#include "qdinputlinebutton.hpp"

class QD_NodeEditGroup: public Fl_Group
{
    private:
        QD_InputLineButton *m_title = nullptr;

    public:
        QD_NodeEditGroup(int, int, int, int, const char * = nullptr);

    public:
        int handle(int) override;
};

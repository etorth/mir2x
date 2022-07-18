#include "qdnodeeditgroup.hpp"

QD_NodeEditGroup::QD_NodeEditGroup(int argX, int argY, int argW, int argH, const char *argLabel)
    : Fl_Group(argX, argY, argW, argH, argLabel)
{
    {
        m_title = new QD_InputLineButton(50, 50, 665, 20, "Title");
    }
    this->end();
}

int QD_NodeEditGroup::handle(int event)
{
    const auto result = Fl_Group::handle(event);
    if(event == FL_PUSH && !result){
        m_title->edit(false);
        return 1;
    }
    return result;
}

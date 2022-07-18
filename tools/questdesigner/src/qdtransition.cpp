#include "qdtransition.hpp"

QD_Transition::QD_Transition(int argX, int argY, int argW, int argH, const char *argLabel)
    : Fl_Group(argX, argY, argW, argH, argLabel)
{
    {   m_enterBox = new Fl_Box(argX + 0, argY + 0, 20, 140, "@>");
        m_enterBox->box(FL_UP_BOX);
    }

    { new Fl_Button(argX + 20, argY + 0, 165, 35, "button");
    }

    { new Fl_Button(argX + 20, argY + 35, 165, 35, "button");
    }

    { new Fl_Button(argX + 20, argY + 70, 165, 35, "button");
    }

    { new Fl_Button(argX + 20, argY + 105, 165, 35, "button");
    }

    { m_callback = new QD_InputMultilineButton(argX + 185, argY + 0, 100, 140, "状态转移逻辑");
    }

    this->end();
}

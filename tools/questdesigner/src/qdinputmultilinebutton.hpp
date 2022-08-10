#pragma once
#include "qdbaseinputbutton.hpp"

class QD_InputMultilineButton: public QD_BaseInputButton
{
    public:
        QD_InputMultilineButton(int argX, int argY, int argW, int argH, const char *argLabel = nullptr)
            : QD_BaseInputButton(argX, argY, argW, argH, argLabel)
        {
            create_widget(true);
        }
};

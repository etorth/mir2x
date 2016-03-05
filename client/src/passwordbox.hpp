#pragma once
#include "inputbox.hpp"

class PasswordBox: public InputBox
{
    public:
        PasswordBox(int, int, uint8_t, uint8_t, uint32_t);
        ~PasswordBox() = default;

    protected:
        void Compile();
};

#pragma once
#include "idbox.hpp"

class PasswordBox: public IDBox
{
    public:
        PasswordBox(int, int, uint8_t, uint8_t, uint32_t);
        ~PasswordBox() = default;

    protected:
        void Compile();
};

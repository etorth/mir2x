#pragma once
#include "inputbox.hpp"

class PasswordBox: public InputBox
{
    public:
        PasswordBox(int, int, const FONTINFO &, const SDL_Color &);
        ~PasswordBox() = default;

    protected:
        void Compile();
};

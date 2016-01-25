#pragma once
#include <SDL.h>
#include <functional>
#include "widget.hpp"
#include "texturemanager.hpp"

class Button: public Widget
{
    public:
        Button(int, int, int, std::function<void()>);
		Button(int, int, int, int, int, int, int, int, int, std::function<void()>);
        ~Button();

    public:
        void Draw();
        void Update(Uint32);
        bool HandleEvent(SDL_Event &);

    private:
        // 0: normal
        // 1: on
        // 2: pressed
        int                     m_State;
        SDL_Texture            *m_Texture[3];
        std::function<void()>   m_OnClick;
};

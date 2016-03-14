#pragma once

#include <vector>
#include <string>
#include <SDL2/SDL.h>

#include "widget.hpp"
#include "tokenbox.hpp"

class InputBox: public Widget
{
    public:
        InputBox(int, int, uint8_t, uint8_t, uint32_t);
        virtual ~InputBox();

    public:
        const char *Content();
        void SetContent(const char *);

    public:
        void SetFocus();
        bool Focus();

    public:
        void Draw();
        void Update(Uint32);
        bool ProcessEvent(SDL_Event &);

    protected:
        virtual void Compile();

    protected:
        void SetProperH();
        void DrawCursor();
        void DrawSystemCursor();
        void PushBack(TOKENBOX &);
        void ResetShowStartX();
        void SetTokenBoxStartX();
        void BindCursorTokenBox(int, int);
        void LoadUTF8CharBoxCache(TOKENBOX &);

    protected:
        int     m_SystemCursorX;
        int     m_SystemCursorY;
        bool    m_DrawOwnSystemCursor;
        int     m_BindTokenBoxIndex;
        int     m_ShowStartX;
        Uint32  m_Ticks;
        bool    m_Focus;

    protected:
        std::vector<TOKENBOX>       m_Line;
        std::string                 m_Content;

    public:
        static int  m_ShowSystemCursorCount;
        static int  m_InputBoxCount;
};

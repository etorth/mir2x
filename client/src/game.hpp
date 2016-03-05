#pragma once 
#include <SDL2/SDL.h>

class Game
{
    private:
        std::atomic<bool>   m_LoginOK;


    public:
        Game();
        ~Game();

    public:
        void Init();
        void MainLoop();

    public:
        enum{
            PROCESSID_NULL   = 0,
            PROCESSID_LOGO   = 1,
            PROCESSID_LOGIN  = 2,
            PROCESSID_RUN    = 3,
            PROCESSID_EXIT   = 4,
        };

    public:
        // since the FontexDB and EmoticonDB are local inside of stGame
        void LoadTokenBoard(TokenBoard *, const tinyxml2::XMLDocument *);

    private:
        void SwitchProcess(int, int);


    private:
        ProcessLogin    *m_ProcessLogin;
        ProcessLogo     *m_ProcessLogo;

    private:
        int     *m_CurrentProcessID;
        Process *m_CurrentProcess;

    private:
        // private utility functions delcared as inline
        //
        inline void SetRenderDefaultColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
        {
            SDL_SetRenderDrawColor(m_Renderer, r, g, b, a);
        }

        inline void RenderClear()
        {
            SDL_RenderClear(m_Renderer);
        }

        inline void RenderPresent()
        {
            SDL_RenderPresent(m_Renderer);
        }

        inline int WindowSizeW()
        {
            int nW;
            SDL_GetWindowSize(m_Window, &nW, nullptr);
            return nW;
        }

        inline int WindowSizeH()
        {
            int nH;
            SDL_GetWindowSize(m_Window, &nH, nullptr);
            return nH;
        }
};

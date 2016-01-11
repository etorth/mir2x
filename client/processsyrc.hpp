/*
 * =====================================================================================
 *
 *       Filename: processsyrc.hpp
 *        Created: 8/14/2015 2:47:30 PM
 *  Last Modified: 09/03/2015 3:13:49 AM
 *
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */
#pragma once

#include <SDL.h>
#include "label.hpp"
#include "process.hpp"
#include "texturemanager.hpp"
#include "tokenboard.hpp"
#include "message.hpp"

class ProcessSyrc: public Process
{
    private:
        SDL_Texture    *m_TextureBackground;
        SDL_Texture    *m_TextureProgressBar;
        int             m_FrameCount;
        int             m_Ratio;

    private:
        Label       m_Info;
        // Label       m_ConnectSucceed;
        // Label      *m_CurrentLabel;
        // TokenBoard  m_TokenBoardConnect;
        // TokenBoard  m_TokenBoardConnectSucceed;
        // TokenBoard *m_CurrentTokenBoard;
    public:
        ProcessSyrc(Game *);
        virtual ~ProcessSyrc();
    public:
        void Enter();
        void Exit();
        void Update();
        void Draw();
        void HandleEvent(SDL_Event *);
    private:
        void HandleMessage(const Message &);
};

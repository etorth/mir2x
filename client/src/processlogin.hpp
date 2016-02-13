/*
 * =====================================================================================
 *
 *       Filename: processlogin.hpp
 *        Created: 8/14/2015 2:47:30 PM
 *  Last Modified: 01/14/2016 06:34:32
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
#include "process.hpp"
#include "texturemanager.hpp"
#include "tokenboard.hpp"
#include "inputbox.hpp"
#include "passwordbox.hpp"
#include "message.hpp"
#include "button.hpp"

class ProcessLogin: public Process
{
    private:
        SDL_Texture    *m_TextureBackground1;
        SDL_Texture    *m_TextureBackground2;
        SDL_Texture    *m_FrameBox;
        Button          m_Button1;
        Button          m_Button2;
        Button          m_Button3;
        Button          m_Button4;
        InputBox        m_IDInputBox;
        PasswordBox     m_PasswordBox;

    public:
        ProcessLogin(Game *);
        ~ProcessLogin();

    public:
        void Enter();
        void Exit();
        void Update();
        void Draw();
        void HandleEvent(SDL_Event *);

    private:
        void HandleMessage(const Message &);

    private:
        void DoLogin();
};

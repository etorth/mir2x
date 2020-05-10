/*
 * =====================================================================================
 *
 *       Filename: processlogin.hpp
 *        Created: 08/14/2015 02:47:30 PM
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

#include <cstdint>
#include <SDL2/SDL.h>

#include "process.hpp"
#include "message.hpp"
#include "inputline.hpp"
#include "labelboard.hpp"
#include "passwordbox.hpp"
#include "tritexbutton.hpp"

class ProcessLogin: public Process
{
    private:
        TritexButton    m_button1;
        TritexButton    m_button2;
        TritexButton    m_button3;
        TritexButton    m_button4;

        InputLine       m_idBox;
        PasswordBox     m_passwordBox;

    private:
        LabelBoard m_buildSignature;

    public:
        ProcessLogin();
        virtual ~ProcessLogin() = default;

    public:
        int ID() const
        {
            return PROCESSID_LOGIN;
        }

    public:
        void Update(double);
        void Draw();
        void processEvent(const SDL_Event &);

    private:
        void DoLogin();
        void DoCreateAccount();
};

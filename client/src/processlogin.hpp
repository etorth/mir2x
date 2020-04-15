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
        TritexButton    m_Button1;
        TritexButton    m_Button2;
        TritexButton    m_Button3;
        TritexButton    m_Button4;

        InputLine       m_idBox;
        PasswordBox     m_PasswordBox;

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

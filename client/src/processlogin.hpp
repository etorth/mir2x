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
        tritexButton    m_Button1;
        tritexButton    m_Button2;
        tritexButton    m_Button3;
        tritexButton    m_Button4;

        inputLine       m_idBox;
        passwordBox     m_PasswordBox;

    private:
        labelBoard m_buildSignature;

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

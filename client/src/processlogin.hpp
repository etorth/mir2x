/*
 * =====================================================================================
 *
 *       Filename: processlogin.hpp
 *        Created: 08/14/2015 02:47:30 PM
 *  Last Modified: 07/17/2017 17:29:28
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

#include <cstdint>
#include <SDL2/SDL.h>

#include "idbox.hpp"
#include "process.hpp"
#include "message.hpp"
#include "passwordbox.hpp"
#include "tritexbutton.hpp"

class ProcessLogin: public Process
{
    private:
        TritexButton    m_Button1;
        TritexButton    m_Button2;
        TritexButton    m_Button3;
        TritexButton    m_Button4;

        IDBox           m_IDBox;
        PasswordBox     m_PasswordBox;

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
        void ProcessEvent(const SDL_Event &);

    private:
        void DoLogin();
};

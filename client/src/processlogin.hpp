/*
 * =====================================================================================
 *
 *       Filename: processlogin.hpp
 *        Created: 08/14/2015 02:47:30 PM
 *  Last Modified: 04/03/2016 17:42:44
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
#include "process.hpp"
#include "idbox.hpp"
#include "passwordbox.hpp"
#include "message.hpp"
#include "button.hpp"

class ProcessLogin: public Process
{
    private:
        Button          m_Button1;
        Button          m_Button2;
        Button          m_Button3;
        Button          m_Button4;

        IDBox           m_IDBox;
        PasswordBox     m_PasswordBox;

        InputBoard      m_InputBoard;

    public:
        ProcessLogin();
        virtual ~ProcessLogin() = default;

    public:
        int ID()
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

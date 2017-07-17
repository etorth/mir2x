/*
 * =====================================================================================
 *
 *       Filename: processlogin.cpp
 *        Created: 08/14/2015 02:47:49
 *  Last Modified: 07/16/2017 20:40:29
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

#include <cstring>
#include <iostream>
#include <algorithm>

#include "log.hpp"
#include "game.hpp"
#include "message.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"
#include "processlogin.hpp"

ProcessLogin::ProcessLogin()
	: Process()
	, m_Button1(150, 482, 0X00000005, [](){})
	, m_Button2(352, 482, 0X00000008, [](){})
	, m_Button3(554, 482, 0X0000000B, [](){ exit(0); })
    , m_Button4(600, 536, 0X0000000E, [this](){ DoLogin(); })
	, m_IDBox(
            159,
            540,
            146,
            18,
            2,
            0,
            14,
            {0XFF, 0XFF, 0XFF, 0XFF},
            {0XFF, 0XFF, 0XFF, 0XFF},
            [this]() -> void
            {
                m_IDBox      .Focus(false);
                m_PasswordBox.Focus(true);
            },
            [this]() -> void
            {
                DoLogin();
            })
	, m_PasswordBox(
            409,
            540,
            146,
            18,
            true,
            2,
            0,
            14,
            {0XFF, 0XFF, 0XFF, 0XFF},
            {0XFF, 0XFF, 0XFF, 0XFF},
            [](){},
            [this]() -> void
            {
                DoLogin();
            })
{}

void ProcessLogin::Update(double fMS)
{
    m_IDBox.Update(fMS);
    m_PasswordBox.Update(fMS);
}

void ProcessLogin::Draw()
{
    extern SDLDevice *g_SDLDevice;
    extern PNGTexDBN *g_ProgUseDBN;

    g_SDLDevice->ClearScreen();

    g_SDLDevice->DrawTexture(g_ProgUseDBN->Retrieve(0X00000003),   0,  75);
    g_SDLDevice->DrawTexture(g_ProgUseDBN->Retrieve(0X00000004),   0, 465);
    g_SDLDevice->DrawTexture(g_ProgUseDBN->Retrieve(0X00000011), 103, 536);

    m_Button1.Draw();
    m_Button2.Draw();
    m_Button3.Draw();
    m_Button4.Draw();

    m_IDBox      .Draw();
    m_PasswordBox.Draw();

    g_SDLDevice->Present();
}

void ProcessLogin::ProcessEvent(const SDL_Event &rstEvent)
{
    m_Button1    .ProcessEvent(rstEvent, nullptr);
    m_Button2    .ProcessEvent(rstEvent, nullptr);
    m_Button3    .ProcessEvent(rstEvent, nullptr);
    m_Button4    .ProcessEvent(rstEvent, nullptr);
    m_IDBox      .ProcessEvent(rstEvent, nullptr);
    m_PasswordBox.ProcessEvent(rstEvent, nullptr);
}

void ProcessLogin::DoLogin()
{
    if(m_IDBox.Content() && m_PasswordBox.Content()){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_INFO, "login account: (%s:%s)", m_IDBox.Content(), m_PasswordBox.Content());

        std::string szID  = m_IDBox.Content();
        std::string szPWD = m_PasswordBox.Content();

        CMLogin stCML;
        std::memset(&stCML, 0, sizeof(stCML));

        if((szID.size() >= sizeof(stCML.ID)) || (szPWD.size() >= sizeof(stCML.Password))){
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_WARNING, "Too long ID/PWD provided");
            return;
        }

        std::memcpy(stCML.ID, szID.c_str(), szID.size());
        std::memcpy(stCML.Password, szPWD.c_str(), szPWD.size());

        extern Game *g_Game;
        g_Game->Send(CM_LOGIN, stCML);
    }
}

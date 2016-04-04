/*
 * =====================================================================================
 *
 *       Filename: processlogin.cpp
 *        Created: 08/14/2015 02:47:49
 *  Last Modified: 04/03/2016 17:58:53
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

#include <iostream>
#include "game.hpp"
#include <cstring>
#include <algorithm>
#include "message.hpp"
#include "processlogin.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"

ProcessLogin::ProcessLogin()
	: Process()
	, m_Button1(150, 482, 255,  5)
	, m_Button2(352, 482, 255,  8)
	, m_Button3(554, 482, 255, 11, [](){ exit(0); })
    , m_Button4(600, 536, 255, 14, [this](){ DoLogin(); })
	, m_IDBox(
            159, 540, 146, 14,       2, 0, 14, {0XFF, 0XFF, 0XFF, 0XFF}, {0XFF, 0XFF, 0XFF, 0XFF})
	, m_PasswordBox(
            409, 540, 146, 14, true, 2, 0, 14, {0XFF, 0XFF, 0XFF, 0XFF}, {0XFF, 0XFF, 0XFF, 0XFF})
    , m_InputBoard(
            100, 100, 300, 200, true, 300, 0, 2, {0XFF, 0XFF, 0X00, 0XFF}, 1, 20, 0, {0XFF, 0X00, 0X00, 0XFF})
{
}

void ProcessLogin::Update(double fMS)
{
    m_IDBox.Update(fMS);
}

void ProcessLogin::Draw()
{
    extern SDLDevice *g_SDLDevice;
    extern PNGTexDBN *g_PNGTexDBN;

    g_SDLDevice->ClearScreen();

    g_SDLDevice->DrawTexture(g_PNGTexDBN->Retrieve(255,  3),   0,  75);
    g_SDLDevice->DrawTexture(g_PNGTexDBN->Retrieve(255,  4),   0, 465);
    g_SDLDevice->DrawTexture(g_PNGTexDBN->Retrieve(255, 17), 103, 536);

    m_Button1.Draw();
    m_Button2.Draw();
    m_Button3.Draw();
    m_Button4.Draw();

    m_IDBox.Draw();
    m_PasswordBox.Draw();

    g_SDLDevice->PushColor(0X00, 0XFF, 0X00, 0XFF);
    g_SDLDevice->DrawRectangle(100, 100, 300, 200);
    g_SDLDevice->PopColor();

    m_InputBoard.Draw();

    g_SDLDevice->Present();
}

void ProcessLogin::ProcessEvent(const SDL_Event &rstEvent)
{
    // m_IDInputBox.ProcessEvent(rstEvent);
    // m_PasswordBox.ProcessEvent(rstEvent);

    bool bValid = true;
    if(false
            || m_Button1.ProcessEvent(rstEvent, &bValid)
            || m_Button2.ProcessEvent(rstEvent, &bValid)
            || m_Button3.ProcessEvent(rstEvent, &bValid)
            || m_Button4.ProcessEvent(rstEvent, &bValid)){
        return;
    }

    if(m_InputBoard.ProcessEvent(rstEvent, &bValid)){
        return;
    }

    if(m_IDBox.ProcessEvent(rstEvent, &bValid)){
        return;
    }

    if(m_PasswordBox.ProcessEvent(rstEvent, &bValid)){
        return;
    }

    switch(rstEvent.type){
        case SDL_KEYDOWN:
            {
                break;
            }
        default:
            break;
    }
}

void ProcessLogin::DoLogin()
{
    std::string szID = m_IDBox.Print(false);
    std::cout << szID << std::endl;
}

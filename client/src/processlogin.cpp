/*
 * =====================================================================================
 *
 *       Filename: processlogin.cpp
 *        Created: 08/14/2015 02:47:49
 *  Last Modified: 04/01/2016 23:38:11
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
    , m_Button4(600, 536, 255, 14)
	, m_IDBox(159, 540, 146, 14, 2, 0, 14, {0XFF, 0XFF, 0XFF, 0XFF}, {0XFF, 0XFF, 0XFF, 0XFF})
	// , m_PasswordBox(146, 14, {0, 14, 0}, {200, 200, 200, 128})
{
    // m_IDInputBox.SetX(159);
    // m_IDInputBox.SetY(540);
    //
    // m_PasswordBox.SetX(409);
    // m_PasswordBox.SetY(540);
}

void ProcessLogin::Update(double)
{
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

    // m_IDInputBox.Draw();
    // m_PasswordBox.Draw();
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

    switch(rstEvent.type){
        case SDL_KEYDOWN:
            {
                break;
            }
        default:
            break;
    }
}

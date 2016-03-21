/*
 * =====================================================================================
 *
 *       Filename: processlogin.cpp
 *        Created: 08/14/2015 02:47:49
 *  Last Modified: 03/20/2016 21:13:50
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
	, m_Button1(255,  5, 150, 482, [](){})
	, m_Button2(255,  8, 352, 482, [](){})
	, m_Button3(255, 11, 554, 482, [](){ exit(0); })
    , m_Button4(255, 14, 600, 536, [this](){})
	// , m_IDInputBox(146, 14, {0, 14, 0}, {200, 200, 200, 128})
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

    if(false
            || m_Button1.ProcessEvent(rstEvent)
            || m_Button2.ProcessEvent(rstEvent)
            || m_Button3.ProcessEvent(rstEvent)
            || m_Button4.ProcessEvent(rstEvent)){
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

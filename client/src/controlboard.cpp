/*
 * =====================================================================================
 *
 *       Filename: controlboard.cpp
 *        Created: 08/21/2016 04:12:57
 *  Last Modified: 07/06/2017 18:32:52
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

#include <stdexcept>
#include <algorithm>
#include <functional>

#include "log.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"
#include "processrun.hpp"
#include "controlboard.hpp"

ControlBoard::ControlBoard(int nX, int nY, Widget *pWidget, bool bAutoDelete)
    : Widget(nX, nY, 0, 0, pWidget, bAutoDelete)
    , m_ProcessRun(nullptr)
    , m_CmdLine(
            185,
            574,
            343,
            15,
            1,
            ColorFunc::COLOR_WHITE,
            2,
            15,
            0,
            ColorFunc::COLOR_WHITE,
            [    ](){                  },
            [this](){ InputLineDone(); })
{}

void ControlBoard::Update(double fMS)
{
    m_CmdLine.Update(fMS);
}

void ControlBoard::DrawEx(int, int, int, int, int, int)
{
    extern PNGTexDBN *g_ProgUseDBN;
    extern SDLDevice *g_SDLDevice;

    g_SDLDevice->DrawTexture(g_ProgUseDBN->Retrieve(0X02000000), 537, 488);
    g_SDLDevice->DrawTexture(g_ProgUseDBN->Retrieve(0X00000012),   0, 466);
    g_SDLDevice->DrawTexture(g_ProgUseDBN->Retrieve(0X00000013), 178, 448);

    g_SDLDevice->PushColor(0X00, 0XFF, 0X00, 0XFF);
    g_SDLDevice->DrawRectangle(m_CmdLine.X(), m_CmdLine.Y(), m_CmdLine.W(), m_CmdLine.H());
    g_SDLDevice->PopColor();

    m_CmdLine.Draw();
}

bool ControlBoard::ProcessEvent(const SDL_Event &rstEvent, bool *bValid)
{
    if(bValid && !(*bValid)){ return false; }
    if(false
            || m_CmdLine.ProcessEvent(rstEvent, bValid)){ return true; }

    switch(rstEvent.type){
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEMOTION:
        default:
            return false;
    }
    return false;
}

void ControlBoard::InputLineDone()
{
    std::string szRawInput  = m_CmdLine.Content();
    std::string szInputLine = szRawInput.substr(szRawInput.find_first_not_of(" \n\r\t"));

    if(szInputLine.empty()){
    }else{
        switch(szInputLine[0]){
            case '!': // broadcast
                {
                    break;
                }
            case '@': // user command
                {
                    if(m_ProcessRun){
                        m_ProcessRun->UserCommand(szInputLine.c_str() + 1);
                    }
                    break;
                }
            case '$': // lua command for super user
                {
                    if(m_ProcessRun){
                        m_ProcessRun->LuaCommand(szInputLine.c_str() + 1);
                    }
                    break;
                }
            default: // normal talk
                {
                    break;
                }
        }
    }
}

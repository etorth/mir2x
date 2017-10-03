/*
 * =====================================================================================
 *
 *       Filename: controlboard.cpp
 *        Created: 08/21/2016 04:12:57
 *  Last Modified: 10/02/2017 18:54:07
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
#include "dbcomrecord.hpp"
#include "controlboard.hpp"

ControlBoard::ControlBoard(int nX, int nY, int nW, ProcessRun *pRun, Widget *pWidget, bool bAutoDelete)
    : Widget(nX, nY, nW, 135, pWidget, bAutoDelete)
    , m_ProcessRun(pRun)
    , m_CmdLine(
            185,
            108,
            343 + (nW - 800),
            15,
            1,
            ColorFunc::COLOR_WHITE,
            1,
            12,
            0,
            ColorFunc::COLOR_WHITE,
            [    ](){                  },
            [this](){ InputLineDone(); },
            this,
            false)
    , m_LocBoard(0, 0, "", 1, 12, 0, {0XFF, 0X00, 0X00, 0X00})
    , m_LogBoard(
            187,
            18,
            341 + (nW - 800),
            83,
            true,
            true,
            true,
            0,
            0,
            1,
            12,
            0,
            ColorFunc::COLOR_WHITE,
            this,
            false)
{
    if(!pRun){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Invalid ProcessRun provided to ControlBoard()");
    }
}

void ControlBoard::Update(double fMS)
{
    m_CmdLine.Update(fMS);
}

void ControlBoard::DrawEx(int, int, int, int, int, int)
{
    // for texture 0X00000012 and 0X00000013
    // I split it into many parts to fix different screen size
    // for screen width is not 800 we build a new interface using these two
    //
    // 0X00000012 : 800 x 134
    // 0X00000013 : 456 x 152
    //
    //                        +------------+                           ---
    //                         \          /                             ^
    // +------+==---------------+        +----------------==+--------+  |  ---
    // |      $                                        +---+$        | 152  | ---
    // |      |                                        |   ||        |  |  134 | 120 as underlay
    // |      |                                        +---+|        |  V   |  |
    // +------+---------------------------------------------+--------+ --- -- ---
    // ^      ^    ^          ^            ^          ^     ^        ^
    // | 178  | 50 |    110   |    127     |    50    | 119 |   166  | = 800
    //
    // |---fixed---|--repeat--|----set-----|-----------fixed---------|

    extern SDLDevice *g_SDLDevice;
    extern PNGTexDBN *g_ProgUseDBN;

    int nX0 = X();
    int nY0 = Y();
    int nW0 = W();

    // draw black underlay for the LogBoard and actor face
    g_SDLDevice->PushColor(0X00, 0X00, 0X00, 0XFF);
    g_SDLDevice->FillRectangle(nX0 + 178 + 2, nY0 + 14, nW0 - (178 + 2) - (166 + 2), 120);
    g_SDLDevice->PopColor();

    // draw command line and log board
    m_CmdLine.Draw();
    m_LogBoard.Draw();

    {
        g_SDLDevice->PushColor(0X00, 0XFF, 0X00, 0XF0);
        g_SDLDevice->DrawRectangle(m_LogBoard.X(), m_LogBoard.Y(), m_LogBoard.W(), m_LogBoard.H());
        g_SDLDevice->PopColor();
    }

    // draw left and right part
    if(auto pTexture = g_ProgUseDBN->Retrieve(0X00000012)){
        g_SDLDevice->DrawTexture(pTexture,             nX0, nY0,   0, 0, 178, 134);
        g_SDLDevice->DrawTexture(pTexture, nW0 - 166 + nX0, nY0, 634, 0, 166, 134);
    }

    // draw middle part
    if(auto pTexture = g_ProgUseDBN->Retrieve(0X00000013)){
        g_SDLDevice->DrawTexture(pTexture,                  178, nY0 - 18,              0, 0,       50, 152);
        g_SDLDevice->DrawTexture(pTexture, nW0 - 166 - 119 - 50, nY0 - 18, 50 + 110 + 127, 0, 50 + 119, 152);

        int nFillRept = (nW0 - (178 + 50) - (50 + 119 + 166)) / 110;
        int nFillRest = (nW0 - (178 + 50) - (50 + 119 + 166)) % 110;
        for(int nIndex = 0; nIndex < nFillRept; ++nIndex){
            g_SDLDevice->DrawTexture(pTexture, 178 + 50 + nIndex * 110, nY0 - 18, 50, 0, 110, 152);
        }

        g_SDLDevice->DrawTexture(pTexture, 178 + 50 + nFillRept * 110, nY0 - 18, 50, 0, nFillRest, 152);
        g_SDLDevice->DrawTexture(pTexture, (nW0 - 178 - 166 - 127) / 2 + 178, nY0 - 18, 50 + 110, 0, 127, 152);
    }

    // draw HP and MP texture
    {
        auto pHP = g_ProgUseDBN->Retrieve(0X00000018);
        auto pMP = g_ProgUseDBN->Retrieve(0X00000019);

        if(pHP && pMP){ 

            // we need to call query
            // so need to validate two textures here

            int nHPH = -1;
            int nHPW = -1;
            int nMPH = -1;
            int nMPW = -1;

            SDL_QueryTexture(pHP, nullptr, nullptr, &nHPW, &nHPH);
            SDL_QueryTexture(pMP, nullptr, nullptr, &nMPW, &nMPH);

            if(auto pMyHero = m_ProcessRun->GetMyHero()){
                double fHPRatio = (pMyHero->HPMax() > 0) ? ((1.0 * pMyHero->HP()) / pMyHero->HPMax()) : 1.0;
                double fMPRatio = (pMyHero->MPMax() > 0) ? ((1.0 * pMyHero->MP()) / pMyHero->MPMax()) : 1.0;

                fHPRatio = std::max<double>(std::min<double>(fHPRatio, 1.0), 0.0);
                fMPRatio = std::max<double>(std::min<double>(fMPRatio, 1.0), 0.0);

                double fLostHPRatio = 1.0 - fHPRatio;
                double fLostMPRatio = 1.0 - fMPRatio;

                auto nLostHPH = (int)(std::lround(nHPH * fLostHPRatio));
                auto nLostMPH = (int)(std::lround(nMPH * fLostMPRatio));

                g_SDLDevice->DrawTexture(pHP, nX0 + 33, nY0 + 8 + nLostHPH, 0, nLostHPH, nHPW, nHPH - nLostHPH);
                g_SDLDevice->DrawTexture(pMP, nX0 + 73, nY0 + 8 + nLostMPH, 0, nLostMPH, nMPW, nMPH - nLostMPH);
            }
        }
    }

    // draw current creature face
    if(auto pTexture = g_ProgUseDBN->Retrieve(m_ProcessRun->GetFocusFaceKey())){
        g_SDLDevice->DrawTexture(pTexture, nX0 + (nW0 - 266), nY0 + 17);
    }

    {
        auto nX = m_ProcessRun->GetMyHero()->X();
        auto nY = m_ProcessRun->GetMyHero()->Y();
        m_LocBoard.SetText(u8"%s: %d %d", DBCOM_MAPRECORD(m_ProcessRun->MapID()).Name, nX, nY);
        m_LocBoard.DrawEx(nX0 + (136 - m_LocBoard.W()) / 2, nY0 + 108, 0, 0, m_LocBoard.W(), m_LocBoard.H());
    }

    // g_SDLDevice->PushColor(0X00, 0XFF, 0X00, 0XFF);
    // g_SDLDevice->DrawRectangle(m_CmdLine.X(), m_CmdLine.Y(), m_CmdLine.W(), m_CmdLine.H());
    // g_SDLDevice->PopColor();
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
    std::string szRealInput;
    std::string szFullInput = m_CmdLine.Content();

    auto nInputPos = szFullInput.find_first_not_of(" \n\r\t");
    szRealInput = (nInputPos == std::string::npos) ? "" : szFullInput.substr(nInputPos);

    if(szRealInput.empty()){
    }else{
        switch(szRealInput[0]){
            case '!': // broadcast
                {
                    break;
                }
            case '@': // user command
                {
                    if(m_ProcessRun){
                        m_ProcessRun->UserCommand(szRealInput.c_str() + 1);
                    }
                    break;
                }
            case '$': // lua command for super user
                {
                    if(m_ProcessRun){
                        m_ProcessRun->LuaCommand(szRealInput.c_str() + 1);
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

void ControlBoard::AddLog(int nLogType, const char *szLog)
{
    if(szLog){
        char szXML[1024];
        switch(nLogType){
            case 0  : std::sprintf(szXML, "<ROOT><OBJECT                                    >%s</OBJECT></ROOT>", szLog); break;
            case 1  : std::sprintf(szXML, "<ROOT><OBJECT BACKCOLOR=\"YELLOW\" COLOR=\"BLUE\">%s</OBJECT></ROOT>", szLog); break;
            default : std::sprintf(szXML, "<ROOT><OBJECT BACKCOLOR=\"RED\"                  >%s</OBJECT></ROOT>", szLog); break;
        }

        m_LogBoard.AddXML(szXML, {});
    }
}

bool ControlBoard::CheckMyHeroMoved()
{
    return true;
}

/*
 * =====================================================================================
 *
 *       Filename: controlboard.cpp
 *        Created: 08/21/2016 04:12:57
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
#include "toll.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "controlboard.hpp"

// for texture 0X00000012 and 0X00000013
// I split it into many parts to fix different screen size
// for screen width is not 800 we build a new interface using these two
//
// 0X00000012 : 800 x 133:  left and right
// 0X00000013 : 456 x 131:  middle log
// 0X00000022 : 127 x 41 :  title
//
//                         +-----------+                           ---
//                          \  title  /                             ^
// +------+==----------------+       +----------------==+--------+  |  ---
// |      $                                        +---+$        | 152  | ---
// |      |                                        |   ||        |  |  133 | 120 as underlay log
// |      |                                        +---+|        |  V   |  |
// +------+---------------------------------------------+--------+ --- -- ---
// ^      ^    ^           ^           ^          ^     ^        ^
// | 178  | 50 |    110    |   127     |    50    | 119 |   166  | = 800
//
// |---fixed---|-------------repeat---------------|---fixed------|

extern Log *g_Log;
extern PNGTexDB *g_ProgUseDB;
extern SDLDevice *g_SDLDevice;

controlBoard::controlBoard(int nX, int nY, int nW, ProcessRun *pRun, Widget *pWidget, bool bAutoDelete)
    : Widget(nX, nY, nW, 133, pWidget, bAutoDelete)
    , m_ProcessRun(pRun)
    , m_ButtonClose
      {
          8,
          72,
          {0XFFFFFFFF, 0X0000001E, 0X0000001F},
          [](){},
          [](){},
          0,
          0,
          0,
          0,
          true,
          this,
          false,
      }
    , m_ButtonMinize
      {
          109,
          72,
          {0XFFFFFFFF, 0X00000020, 0X00000021},
          [](){},
          [](){},
          0,
          0,
          0,
          0,
          true,
          this,
          false,
      }
    , m_ButtonInventory
      {
          682,
          33,
          {0XFFFFFFFF, 0X00000030, 0X00000031},
          [](){},
          [this]()
          {
              if(auto pInventory = m_ProcessRun->GetWidget("InventoryBoard")){
                  pInventory->Show(!pInventory->Show());
              }
          },
          0,
          0,
          0,
          0,
          true,
          this,
          false,
      }
    , m_CmdLine(
            185,
            108,
            343 + (nW - 800),
            15,
            1,
            {0XFF, 0XFF, 0XFF, 0XFF},
            1,
            12,
            0,
            {0XFF, 0XFF, 0XFF, 0XFF},
            [    ](){                  },
            [this](){ InputLineDone(); },
            this,
            false)
    , m_LocBoard(0, 0, "", 1, 12, 0, ColorFunc::RGBA(0XFF, 0X00, 0X00, 0X00))
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
            {0XFF, 0XFF, 0XFF, 0XFF},
            this,
            false)
{
    if(!pRun){
        throw fflerror("invalid ProcessRun provided to controlBoard()");
    }

    auto fnAssertImage = [](uint32_t img, int w, int h)
    {
        if(auto ptex = g_ProgUseDB->Retrieve(img)){
            int readw = -1;
            int readh = -1;
            if(!SDL_QueryTexture(ptex, 0, 0, &readw, &readh)){
                if(w == readw && h == readh){
                    return;
                }
            }
        }
        throw fflerror("image assertion failed: img = %llu, w = %d, h = %d", to_LLU(img), w, h);
    };

    fnAssertImage(0X00000012, 800, 133);
    fnAssertImage(0X00000013, 456, 131);
    fnAssertImage(0X00000022, 127,  41);

    if(X() != 0 || Y() + H() != g_SDLDevice->WindowH(false) || W() != g_SDLDevice->WindowW(false)){
        throw fflerror("controlBoard has wrong location or size");
    }
}

void controlBoard::Update(double fMS)
{
    m_CmdLine.Update(fMS);
}

void controlBoard::drawLeft()
{
    const int nY0 = Y();

    // draw left part
    if(auto pTexture = g_ProgUseDB->Retrieve(0X00000012)){
        g_SDLDevice->DrawTexture(pTexture, 0, nY0, 0, 0, 178, 133);
    }

    // draw HP and MP texture
    {
        auto pHP = g_ProgUseDB->Retrieve(0X00000018);
        auto pMP = g_ProgUseDB->Retrieve(0X00000019);

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

                fHPRatio = (std::max<double>)((std::min<double>)(fHPRatio, 1.0), 0.0);
                fMPRatio = (std::max<double>)((std::min<double>)(fMPRatio, 1.0), 0.0);

                double fLostHPRatio = 1.0 - fHPRatio;
                double fLostMPRatio = 1.0 - fMPRatio;

                auto nLostHPH = (int)(std::lround(nHPH * fLostHPRatio));
                auto nLostMPH = (int)(std::lround(nMPH * fLostMPRatio));

                g_SDLDevice->DrawTexture(pHP, 33, nY0 + 9 + nLostHPH, 0, nLostHPH, nHPW, nHPH - nLostHPH);
                g_SDLDevice->DrawTexture(pMP, 73, nY0 + 9 + nLostMPH, 0, nLostMPH, nMPW, nMPH - nLostMPH);
            }
        }
    }

    // draw current location
    {
        const auto nX = m_ProcessRun->GetMyHero()->X();
        const auto nY = m_ProcessRun->GetMyHero()->Y();
        m_LocBoard.setText(u8"%s: %d %d", DBCOM_MAPRECORD(m_ProcessRun->MapID()).Name, nX, nY);
        m_LocBoard.drawEx((136 - m_LocBoard.W()) / 2, nY0 + 109, 0, 0, m_LocBoard.W(), m_LocBoard.H());
    }
}

void controlBoard::drawRight()
{
    const int nY0 = Y();
    const int nW0 = W();

    // draw right part
    if(auto pTexture = g_ProgUseDB->Retrieve(0X00000012)){
        g_SDLDevice->DrawTexture(pTexture, nW0 - 166, nY0, 800 - 166, 0, 166, 133);
    }

    m_ButtonClose    .Draw();
    m_ButtonMinize   .Draw();
    m_ButtonInventory.Draw();
}

std::tuple<int, int> controlBoard::scheduleStretch() const
{
    // use same way for default or expand mode
    // this requires texture 0X00000013 and 0X00000027 are of width 456

    const int drawW   = W() - 50 - 119 - 178 - 166;
    const int repeatW = 456 - 50 - 119;

    if(drawW < repeatW){
        return {0, drawW};
    }

    if(drawW % repeatW == 0){
        return {drawW / repeatW, 0};
    }

    const double fillRatio = (1.0 * (drawW % repeatW)) / repeatW;
    if(fillRatio < 0.5){
        return {drawW / repeatW - 1, repeatW + (drawW % repeatW)};
    }
    return {drawW / repeatW, drawW % repeatW};
}

void controlBoard::drawMiddleDefalt()
{
    const int nY0 = Y();
    const int nW0 = W();

    // draw black underlay for the LogBoard and actor face
    {
        SDLDevice::EnableDrawColor enableColor(ColorFunc::RGBA(0X00, 0X00, 0X00, 0XFF));
        g_SDLDevice->FillRectangle(178 + 2, nY0 + 14, nW0 - (178 + 2) - (166 + 2), 120);
    }

    m_CmdLine.Draw();
    m_LogBoard.Draw();

    // draw middle part
    if(auto pTexture = g_ProgUseDB->Retrieve(0X00000013)){
        g_SDLDevice->DrawTexture(pTexture,             178, nY0 + 2,         0, 0,  50, 131);
        g_SDLDevice->DrawTexture(pTexture, nW0 - 166 - 119, nY0 + 2, 456 - 119, 0, 119, 131);

        const int repeatW = 456 - 50 - 119;
        const auto [repeat, stretch] = scheduleStretch();

        for(int i = 0; i < repeat; ++i){
            g_SDLDevice->DrawTexture(pTexture, 178 + 50 + i * repeatW, nY0 + 2, 50, 0, repeatW, 131);
        }

        // for the rest area
        // need to stretch or shrink
        if(stretch > 0){
            g_SDLDevice->DrawTexture(pTexture, 178 + 50 + repeat * repeatW, nY0 + 2, stretch, 131, 50, 0, repeatW, 131);
        }
    }

    // draw current creature face
    if(auto pTexture = g_ProgUseDB->Retrieve(m_ProcessRun->GetFocusFaceKey())){
        g_SDLDevice->DrawTexture(pTexture, nW0 - 266, nY0 + 18);
    }

    // draw title
    if(auto pTexture = g_ProgUseDB->Retrieve(0X00000022)){
        int titleW = -1;
        int titleH = -1;

        SDL_QueryTexture(pTexture, 0, 0, &titleW, &titleH);
        const int titleDstX = 178 + (nW0 - 178 - 116 - titleW) / 2;
        const int titleDstY = nY0 - 19;
        g_SDLDevice->DrawTexture(pTexture, titleDstX, titleDstY);
    }

    // g_SDLDevice->PushColor(0X00, 0XFF, 0X00, 0XFF);
    // g_SDLDevice->DrawRectangle(m_CmdLine.X(), m_CmdLine.Y(), m_CmdLine.W(), m_CmdLine.H());
    // g_SDLDevice->PopColor();

    m_ButtonClose    .Draw();
    m_ButtonMinize   .Draw();
    m_ButtonInventory.Draw();
}

void controlBoard::drawMiddleExpand()
{

}

void controlBoard::drawEx(int, int, int, int, int, int)
{
    drawLeft();

    if(m_expand){
        drawMiddleExpand();
    }
    else{
        drawMiddleDefalt();
    }

    drawRight();
}

bool controlBoard::processEvent(const SDL_Event &rstEvent, bool *bValid)
{
    if(bValid && !(*bValid)){ return false; }
    if(false
            || m_CmdLine        .processEvent(rstEvent, bValid)
            || m_ButtonClose    .processEvent(rstEvent, bValid)
            || m_ButtonMinize   .processEvent(rstEvent, bValid)
            || m_ButtonInventory.processEvent(rstEvent, bValid)){ return true; }

    switch(rstEvent.type){
        case SDL_KEYDOWN:
            {
                switch(rstEvent.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            m_CmdLine.Focus(true);
                            return true;
                        }
                    default:
                        {
                            return false;
                        }
                }
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEMOTION:
        default:
            {
                return false;
            }
    }
    return false;
}

void controlBoard::InputLineDone()
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

void controlBoard::AddLog(int nLogType, const char *szLog)
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

bool controlBoard::CheckMyHeroMoved()
{
    return true;
}

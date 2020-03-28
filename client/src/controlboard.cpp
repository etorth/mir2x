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

// 
// 0X00000027 : 456 x 298: char box frame
//
//                         +-----------+                        ---
//                          \  title  /                          ^
//        +==----------------+       +----------------==+ ---    |  ---- <-- startY
//        $                                             $  ^     |   47
//        |                                             |  |     |  ----
//        |                                             |  |     |   |
//        |                                             |  |     |  196: use to repeat, as m_stretchH
//        |                                             | 298   319  |
//        +---------------------------------------+-----+  |     |  ----
//        |                                       |     |  |     |   55
//        |                                       |() ()|  |     |   |
//        |                                       |     |  v     v   |
// +------+---------------------------------------+-----+--------+ --- -- ---
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
                  pInventory->show(!pInventory->show());
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
    , m_buttonSwitchMode
      {
          nW - 181,
          5,
          {0XFFFFFFFF, 0X00000028, 0X00000029},
          [](){},
          [this]()
          {
              switchExpandMode();
              m_buttonSwitchMode.setOff();
          },
          0,
          0,
          0,
          0,
          true,
          this,
          false,
      }
    , m_level
      {
          0,
          0,
          [this](int dy)
          {
              if(!m_expand){
                  return;
              }

              m_stretchH = std::max<int>(m_stretchH - dy, m_stretchHMin);
              m_stretchH = std::min<int>(m_stretchH, g_SDLDevice->WindowH(false) - 47 - 55);
              setButtonLoc();
          },
          [this]()
          {
              if(!m_expand){
                  switchExpandMode();
                  m_stretchH = g_SDLDevice->WindowH(false) - 47 - 55;
                  setButtonLoc();
                  return;
              }

              if(m_stretchH != m_stretchHMin){
                  m_stretchH = m_stretchHMin;
              }
              else{
                  m_stretchH = g_SDLDevice->WindowH(false) - 47 - 55;
              }
              setButtonLoc();
          },
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
    fnAssertImage(0X00000027, 456, 298);

    if(X() != 0 || Y() + H() != g_SDLDevice->WindowH(false) || W() != g_SDLDevice->WindowW(false)){
        throw fflerror("controlBoard has wrong location or size");
    }

    m_level.setLevel(7);
    m_level.moveTo(178 + (W() - 178 - 166 - m_level.W()) / 2, 6 - m_level.H() / 2);
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

    m_ButtonClose.Draw();
    m_ButtonMinize.Draw();
}

void controlBoard::drawRight()
{
    const int nY0 = Y();
    const int nW0 = W();

    // draw right part
    if(auto pTexture = g_ProgUseDB->Retrieve(0X00000012)){
        g_SDLDevice->DrawTexture(pTexture, nW0 - 166, nY0, 800 - 166, 0, 166, 133);
    }

    m_ButtonInventory.Draw();
}

std::tuple<int, int> controlBoard::scheduleStretch(int dstSize, int srcSize)
{
    // use same way for default or expand mode
    // this requires texture 0X00000013 and 0X00000027 are of width 456

    if(dstSize < srcSize){
        return {0, dstSize};
    }

    if(dstSize % srcSize == 0){
        return {dstSize / srcSize, 0};
    }

    const double fillRatio = (1.0 * (dstSize % srcSize)) / srcSize;
    if(fillRatio < 0.5){
        return {dstSize / srcSize - 1, srcSize + (dstSize % srcSize)};
    }
    return {dstSize / srcSize, dstSize % srcSize};
}

void controlBoard::drawMiddleDefault()
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
        const int drawW   = nW0 - 50 - 119 - 178 - 166;

        const auto [repeat, stretch] = scheduleStretch(drawW, repeatW);
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
        const int titleDstX = 178 + (nW0 - 178 - 166 - titleW) / 2;
        const int titleDstY = nY0 - 19;
        g_SDLDevice->DrawTexture(pTexture, titleDstX, titleDstY);
    }

    // g_SDLDevice->PushColor(0X00, 0XFF, 0X00, 0XFF);
    // g_SDLDevice->DrawRectangle(m_CmdLine.X(), m_CmdLine.Y(), m_CmdLine.W(), m_CmdLine.H());
    // g_SDLDevice->PopColor();

    m_buttonSwitchMode.Draw();
    m_level.Draw();
}

void controlBoard::drawMiddleExpand()
{
    const int nY0 = Y();
    const int nW0 = W();
    const int nH0 = H();

    // use this position to calculate all points
    // the Y-axis on screen that the big chat-frame starts
    const int startY = nY0 + nH0 - 55 - m_stretchH - 47;

    // draw black underlay for the big log board
    {
        SDLDevice::EnableDrawBlendMode enableDrawBlendMode(SDL_BLENDMODE_BLEND);
        SDLDevice::EnableDrawColor enableColor(ColorFunc::RGBA(0X00, 0X00, 0X00, 0XF0));
        g_SDLDevice->FillRectangle(178 + 2, startY + 2, nW0 - (178 + 2) - (166 + 2), 47 + m_stretchH);
    }

    if(auto pTexture = g_ProgUseDB->Retrieve(0X00000027)){

        // draw four corners
        g_SDLDevice->DrawTexture(pTexture,             178,                   startY,         0,        0,  50, 47);
        g_SDLDevice->DrawTexture(pTexture, nW0 - 166 - 119,                   startY, 456 - 119,        0, 119, 47);
        g_SDLDevice->DrawTexture(pTexture,             178, startY + 47 + m_stretchH,         0, 298 - 55,  50, 55);
        g_SDLDevice->DrawTexture(pTexture, nW0 - 166 - 119, startY + 47 + m_stretchH, 456 - 119, 298 - 55, 119, 55);

        // draw two stretched vertical bars
        const int repeatH = 298 - 47 - 55;
        const auto [repeatHCnt, stretchH] = scheduleStretch(m_stretchH, repeatH);

        for(int i = 0; i < repeatHCnt; ++i){
            g_SDLDevice->DrawTexture(pTexture,             178, startY + 47 + i * repeatH,         0, 47,  50, repeatH);
            g_SDLDevice->DrawTexture(pTexture, nW0 - 166 - 119, startY + 47 + i * repeatH, 456 - 119, 47, 119, repeatH);
        }

        if(stretchH > 0){
            g_SDLDevice->DrawTexture(pTexture,             178, startY + 47 + repeatHCnt * repeatH,  50, stretchH,         0, 47,  50, repeatH);
            g_SDLDevice->DrawTexture(pTexture, nW0 - 166 - 119, startY + 47 + repeatHCnt * repeatH, 119, stretchH, 456 - 119, 47, 119, repeatH);
        }

        // draw horizontal top bar and bottom input area
        const int repeatW = 456 - 50 - 119;
        const int drawW   = nW0 - 50 - 119 - 178 - 166;

        const auto [repeatWCnt, stretchW] = scheduleStretch(drawW, repeatW);
        for(int i = 0; i < repeatWCnt; ++i){
            g_SDLDevice->DrawTexture(pTexture, 178 + 50 + i * repeatW,                   startY, 50,        0, repeatW, 47);
            g_SDLDevice->DrawTexture(pTexture, 178 + 50 + i * repeatW, startY + 47 + m_stretchH, 50, 298 - 55, repeatW, 55);
        }

        if(stretchW > 0){
            g_SDLDevice->DrawTexture(pTexture, 178 + 50 + repeatWCnt * repeatW,                   startY, stretchW, 47, 50,        0, repeatW, 47);
            g_SDLDevice->DrawTexture(pTexture, 178 + 50 + repeatWCnt * repeatW, startY + 47 + m_stretchH, stretchW, 55, 50, 298 - 55, repeatW, 55);
        }
    }

    // draw title
    if(auto pTexture = g_ProgUseDB->Retrieve(0X00000022)){
        int titleW = -1;
        int titleH = -1;

        SDL_QueryTexture(pTexture, 0, 0, &titleW, &titleH);
        const int titleDstX = 178 + (nW0 - 178 - 166 - titleW) / 2;
        const int titleDstY = startY - 2 - 19;
        g_SDLDevice->DrawTexture(pTexture, titleDstX, titleDstY);
    }

    m_buttonSwitchMode.Draw();
    m_level.Draw();
}

void controlBoard::drawEx(int, int, int, int, int, int)
{
    drawLeft();

    if(m_expand){
        drawMiddleExpand();
    }
    else{
        drawMiddleDefault();
    }

    drawRight();
}

bool controlBoard::processEvent(const SDL_Event &event, bool valid)
{
    bool takeEvent = false;

    takeEvent |= m_level           .processEvent(event, valid && !takeEvent);
    takeEvent |= m_CmdLine         .processEvent(event, valid && !takeEvent);
    takeEvent |= m_ButtonClose     .processEvent(event, valid && !takeEvent);
    takeEvent |= m_ButtonMinize    .processEvent(event, valid && !takeEvent);
    takeEvent |= m_ButtonInventory .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonSwitchMode.processEvent(event, valid && !takeEvent);

    if(takeEvent){
        return true;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            m_CmdLine.focus(true);
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

void controlBoard::switchExpandMode()
{
    if(m_expand){
        m_expand = false;
    }
    else{
        m_expand = true;
        m_stretchH = m_stretchHMin;
    }
    setButtonLoc();
}

void controlBoard::setButtonLoc()
{
    // diff of height of texture 0X00000013 and 0X00000027
    // when you draw something on default log board at (X, Y), (0, 0) is left-top
    // if you need to keep the same location on expand log board, draw on(X, Y - modeDiffY)

    const int modeDiffY = (298 - 131) + (m_stretchH - m_stretchHMin);
    if(m_expand){
        m_buttonSwitchMode.moveTo(W() - 181, 5 - modeDiffY);
        m_level.moveTo(178 + (W() - 178 - 166 - m_level.W()) / 2, 6 - m_level.H() / 2 - modeDiffY);
    }
    else{
        m_buttonSwitchMode.moveTo(W() - 181, 5);
        m_level.moveTo(178 + (W() - 178 - 166 - m_level.W()) / 2, 6 - m_level.H() / 2);
    }
}

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
// +------+==----------------+       +----------------==+--------+  |  --- <-- left/right is 133, middle is 131
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

ControlBoard::ControlBoard(int startY, int boardW, ProcessRun *pRun)
    : Widget(0, startY, boardW, 133, nullptr, false)
    , m_processRun(pRun)
    , m_left
      {
          0,
          0,
          178,
          133,
          this,
      }

    , m_middle
      {
          178,
          2, // middle tex height is 131, not 133
          boardW - 178 - 166,
          131,
          this,
      }

    , m_right
      {
          boardW - 166,
          0,
          166,
          133,
          this,
      }
    
    , m_buttonClose
      {
          8,
          72,
          {SYS_TEXNIL, 0X0000001E, 0X0000001F},

          nullptr,
          [](){ std::exit(0); },

          0,
          0,
          0,
          0,

          true,
          &m_left,
      }

    , m_buttonMinize
      {
          109,
          72,
          {SYS_TEXNIL, 0X00000020, 0X00000021},

          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          &m_left,
      }

    , m_buttonInventory
      {
          48,
          33,
          {SYS_TEXNIL, 0X00000030, 0X00000031},

          nullptr,
          [this]()
          {
              if(auto p = m_processRun->getWidget("InventoryBoard")){
                  p->show(!p->show());
              }
          },

          0,
          0,
          0,
          0,

          true,
          &m_right,
      }

    , m_buttonSwitchMode
      {
          boardW - 178 - 181,
          3,
          {SYS_TEXNIL, 0X00000028, 0X00000029},

          nullptr,
          [this]()
          {
              switchExpandMode();
          },

          0,
          0,
          0,
          0,

          true,
          &m_middle,
      }

    , m_buttonEmoji
      {
          boardW - 178 - 260,
          87,
          {SYS_TEXNIL, 0X00000023, 0X00000024},

          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          &m_middle,
      }

    , m_buttonMute
      {
          boardW - 178 - 220,
          87,
          {SYS_TEXNIL, 0X00000025, 0X00000026},

          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          &m_middle,
      }

    , m_levelBox
      {
          0, // need reset
          0, // need reset

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
          &m_middle,
      }

    , m_cmdLine
      {
          7,
          105,
          343 + (boardW - 800),
          17,

          1,
          12,

          0,
          colorf::WHITE,

          2,
          colorf::WHITE,

          nullptr,
          [this]()
          {
              inputLineDone();
          },

          &m_middle,
      }

    , m_locBoard
      {
          0, // need reset
          109,

          "",
          1,
          12,
          0,

          colorf::WHITE,
          &m_left,
      }

    , m_logBoard
      {
          9,
          0, // need reset
          341 + (boardW - 800),
          false,

          {0, 0, 0, 0},
          false,

          1,
          12,
          0,

          colorf::WHITE,
          LALIGN_LEFT,
          0,
          0,

          nullptr,
          &m_middle,
      }
{
    if(!pRun){
        throw fflerror("invalid ProcessRun provided to ControlBoard()");
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
        throw fflerror("image assertion failed: img = %llu, w = %d, h = %d", toLLU(img), w, h);
    };

    fnAssertImage(0X00000012, 800, 133);
    fnAssertImage(0X00000013, 456, 131);
    fnAssertImage(0X00000022, 127,  41);
    fnAssertImage(0X00000027, 456, 298);

    if(x() != 0 || y() + h() != g_SDLDevice->WindowH(false) || w() != g_SDLDevice->WindowW(false)){
        throw fflerror("ControlBoard has wrong location or size");
    }

    m_levelBox.setLevel(7);
    m_levelBox.moveTo((w() - 178 - 166 - m_levelBox.w()) / 2, 4 - m_levelBox.h() / 2);
}

void ControlBoard::update(double fMS)
{
    m_cmdLine.update(fMS);
    m_logBoard.update(fMS);
}

void ControlBoard::drawLeft()
{
    const int nY0 = y();

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

            if(auto pMyHero = m_processRun->GetMyHero()){
                double fHPRatio = (pMyHero->maxHP() > 0) ? ((1.0 * pMyHero->HP()) / pMyHero->maxHP()) : 1.0;
                double fMPRatio = (pMyHero->maxMP() > 0) ? ((1.0 * pMyHero->MP()) / pMyHero->maxMP()) : 1.0;

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
        const int nX = m_processRun->GetMyHero()->x();
        const int nY = m_processRun->GetMyHero()->y();
        m_locBoard.setText(u8"%s: %d %d", DBCOM_MAPRECORD(m_processRun->MapID()).Name, nX, nY);

        const int locBoardStartX = (136 - m_locBoard.w()) / 2;
        m_locBoard.drawEx(locBoardStartX, m_locBoard.y(), 0, 0, m_locBoard.w(), m_locBoard.h());
    }

    m_buttonClose.draw();
    m_buttonMinize.draw();
}

void ControlBoard::drawRight()
{
    const int nY0 = y();
    const int nW0 = w();

    // draw right part
    if(auto pTexture = g_ProgUseDB->Retrieve(0X00000012)){
        g_SDLDevice->DrawTexture(pTexture, nW0 - 166, nY0, 800 - 166, 0, 166, 133);
    }

    m_buttonInventory.draw();
}

std::tuple<int, int> ControlBoard::scheduleStretch(int dstSize, int srcSize)
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

void ControlBoard::drawMiddleDefault()
{
    const int nY0 = y();
    const int nW0 = w();

    // draw black underlay for the logBoard and actor face
    {
        SDLDevice::EnableDrawColor enableColor(colorf::RGBA(0X00, 0X00, 0X00, 0XFF));
        g_SDLDevice->FillRectangle(178 + 2, nY0 + 14, nW0 - (178 + 2) - (166 + 2), 120);
    }

    m_cmdLine.draw();
    drawLogBoardDefault();

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
    if(auto pTexture = g_ProgUseDB->Retrieve(m_processRun->GetFocusFaceKey())){
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

    m_buttonSwitchMode.draw();
    m_levelBox.draw();
}

void ControlBoard::drawLogBoardDefault()
{
    const int dstX = 187;
    const int dstY = logBoardStartY();

    const int srcX = 0;
    const int srcY = std::max<int>(0, m_logBoard.h() - 83);
    const int srcW = m_logBoard.w();
    const int srcH = 83;

    m_logBoard.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}

void ControlBoard::drawLogBoardExpand()
{
    const int dstX = 187;
    const int dstY = logBoardStartY();

    const int boardFrameH = m_stretchH + 47 + 55 - 70;
    const int srcX = 0;
    const int srcY = std::max<int>(0, m_logBoard.h() - boardFrameH);
    const int srcW = m_logBoard.w();
    const int srcH = boardFrameH;

    m_logBoard.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}

void ControlBoard::drawMiddleExpand()
{
    const int nY0 = y();
    const int nW0 = w();
    const int nH0 = h();

    // use this position to calculate all points
    // the Y-axis on screen that the big chat-frame starts
    const int startY = nY0 + nH0 - 55 - m_stretchH - 47;

    // draw black underlay for the big log board
    {
        SDLDevice::EnableDrawBlendMode enableDrawBlendMode(SDL_BLENDMODE_BLEND);
        SDLDevice::EnableDrawColor enableColor(colorf::RGBA(0X00, 0X00, 0X00, 0XF0));
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

    m_buttonSwitchMode.draw();
    m_levelBox.draw();
    m_cmdLine.draw();
    m_buttonEmoji.draw();
    m_buttonMute.draw();
    drawLogBoardExpand();
}

void ControlBoard::drawEx(int, int, int, int, int, int)
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

bool ControlBoard::processEvent(const SDL_Event &event, bool valid)
{
    bool takeEvent = false;

    takeEvent |= m_levelBox        .processEvent(event, valid && !takeEvent);
    takeEvent |= m_cmdLine         .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonClose     .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonMinize    .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonInventory .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonSwitchMode.processEvent(event, valid && !takeEvent);

    if(m_expand){
        takeEvent |= m_buttonEmoji.processEvent(event, valid && !takeEvent);
        takeEvent |= m_buttonMute .processEvent(event, valid && !takeEvent);
    }

    if(takeEvent){
        return true;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            m_cmdLine.focus(true);
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

void ControlBoard::inputLineDone()
{
    const std::string fullInput = m_cmdLine.getRawString();
    const auto inputPos = fullInput.find_first_not_of(" \n\r\t");
    const std::string realInput = (inputPos == std::string::npos) ? "" : fullInput.substr(inputPos);

    m_cmdLine.clear();
    if(realInput.empty()){
        return;
    }

    switch(realInput[0]){
        case '!': // broadcast
            {
                break;
            }
        case '@': // user command
            {
                if(m_processRun){
                    m_processRun->UserCommand(realInput.c_str() + 1);
                }
                break;
            }
        case '$': // lua command for super user
            {
                if(m_processRun){
                    m_processRun->LuaCommand(realInput.c_str() + 1);
                }
                break;
            }
        default: // normal talk
            {
                addLog(0, realInput.c_str());
                break;
            }
    }
}

void ControlBoard::addLog(int, const char *log)
{
    if(!log){
        throw fflerror("null log string");
    }
    m_logBoard.addParXML(m_logBoard.parCount(), {0, 0, 0, 0}, str_printf("<par>%s</par>", log).c_str());
    if(std::rand() % 2 == 0){
        m_logBoard.addParXML(m_logBoard.parCount(), {0, 0, 0, 0}, str_printf("<par>test emoji: <emoji id=\"%d\"/></par>", (int)(std::rand() % 3)).c_str());
    }
}

bool ControlBoard::CheckMyHeroMoved()
{
    return true;
}

void ControlBoard::switchExpandMode()
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

void ControlBoard::setButtonLoc()
{
    // diff of height of texture 0X00000013 and 0X00000027
    // when you draw something on default log board at (X, Y), (0, 0) is left-top
    // if you need to keep the same location on expand log board, draw on(X, Y - modeDiffY)

    const int boardW = w();
    const int modeDiffY = (298 - 131) + (m_stretchH - m_stretchHMin);

    if(m_expand){
        m_buttonSwitchMode.moveTo(boardW - 178 - 181, 3 - modeDiffY);
        m_levelBox.moveTo((boardW - 178 - 166 - m_levelBox.w()) / 2, 4 - m_levelBox.h() / 2 - modeDiffY);

        m_buttonEmoji.moveTo(boardW - 178 - 260, 87);
        m_buttonMute .moveTo(boardW - 178 - 220, 87);
    }
    else{
        m_buttonSwitchMode.moveTo(boardW - 178 - 181, 3);
        m_levelBox.moveTo((boardW - 178 - 166 - m_levelBox.w()) / 2, 4 - m_levelBox.h() / 2);
    }
}

int ControlBoard::logBoardStartY() const
{
    if(!m_expand){
        return g_SDLDevice->WindowH(false) - 120;
    }
    return g_SDLDevice->WindowH(false) - 55 - m_stretchH - 47 + 12; // 12 is texture top-left to log line distane
}

void ControlBoard::resizeWidth(int boardW)
{
    m_right.moveBy(boardW - w(), 0);
    m_w = boardW;

    setButtonLoc();
}

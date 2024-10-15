#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "log.hpp"
#include "colorf.hpp"
#include "totype.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"
#include "processrun.hpp"
#include "controlboard.hpp"
#include "clientmonster.hpp"
#include "teamstateboard.hpp"

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

extern Log *g_log;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ControlBoard::ControlBoard(int boardW, int startY, ProcessRun *proc, Widget *pwidget, bool autoDelete)
    : Widget(DIR_UPLEFT, 0, startY, boardW, 133, {}, pwidget, autoDelete)
    , m_processRun(proc)
    , m_left
      {
          DIR_UPLEFT,
          0,
          0,
          178,
          133,

          {},

          this,
          false,
      }

    , m_middle
      {
          DIR_UPLEFT,
          178,
          2, // middle tex height is 131, not 133
          boardW - 178 - 166,
          131,
          {},
          this,
      }

    , m_right
      {
          DIR_UPLEFT,
          boardW - 166,
          0,
          166,
          133,
          {},
          this,
      }

    , m_buttonQuickAccess
      {
          DIR_UPLEFT,
          148,
          2,
          {SYS_U32NIL, 0X0B000000, 0X0B000001},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              if(auto p = m_processRun->getWidget("QuickAccessBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_left,
      }

    , m_buttonClose
      {
          DIR_UPLEFT,
          8,
          72,
          {SYS_U32NIL, 0X0000001E, 0X0000001F},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [](ButtonBase *)
          {
              std::exit(0);
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_left,
      }

    , m_buttonMinize
      {
          DIR_UPLEFT,
          109,
          72,
          {SYS_U32NIL, 0X00000020, 0X00000021},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_left,
      }

    , m_buttonExchange
      {
          DIR_UPLEFT,
          4,
          6,

          1,
          1,
          10,

          80,
          colorf::WHITE + colorf::A_SHF(255),
          0X00000042,

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              addLog(0, "exchange doesn't implemented yet.");
          },

          true,
          &m_right,
      }

    , m_buttonMiniMap
      {
          DIR_UPLEFT,
          4,
          40,

          1,
          1,
          10,

          80,
          colorf::WHITE + colorf::A_SHF(255),
          0X00000043,

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              if(auto p = dynamic_cast<MiniMapBoard *>(m_processRun->getWidget("MiniMapBoard"))){
                  if(p->getMiniMapTexture()){
                      p->flipMiniMapShow();
                  }
                  else{
                      addLog(CBLOG_ERR, to_cstr(u8"没有可用的地图"));
                  }
              }
          },

          true,
          &m_right,
      }

    , m_buttonMagicKey
      {
          DIR_UPLEFT,
          4,
          75,

          1,
          1,
          10,

          80,
          colorf::WHITE + colorf::A_SHF(255),
          0X00000044,

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              m_processRun->flipDrawMagicKey();
          },

          true,
          &m_right,
      }

    , m_buttonInventory
      {
          DIR_UPLEFT,
          48,
          33,
          {0X00000030, 0X00000030, 0X00000031},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              if(auto p = m_processRun->getWidget("InventoryBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_right,
      }

    , m_buttonHeroState
      {
          DIR_UPLEFT,
          77,
          31,
          {0X00000033, 0X00000033, 0X00000032},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              if(auto p = m_processRun->getWidget("PlayerStateBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_right,
      }

    , m_buttonHeroMagic
      {
          DIR_UPLEFT,
          105,
          33,
          {0X00000035, 0X00000035, 0X00000034},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              if(auto p = m_processRun->getWidget("SkillBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_right,
      }

    , m_buttonGuild
      {
          DIR_UPLEFT,
          40,
          11,
          {0X00000036, 0X00000036, 0X00000037},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              if(auto p = m_processRun->getWidget("GuildBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_right,
      }

    , m_buttonTeam
      {
          DIR_UPLEFT,
          72,
          8,
          {0X00000038, 0X00000038, 0X00000039},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              auto boardPtr = dynamic_cast<TeamStateBoard *>(m_processRun->getWidget("TeamStateBoard"));
              auto  heroPtr = m_processRun->getMyHero();

              if(heroPtr->hasTeam()){
                  boardPtr->flipShow();
                  if(boardPtr->show()){
                      boardPtr->refresh();
                  }
              }
              else{
                  m_processRun->setCursor(ProcessRun::CURSOR_TEAMFLAG);
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_right,
      }

    , m_buttonQuest
      {
          DIR_UPLEFT,
          108,
          11,
          {0X0000003A, 0X0000003A, 0X0000003B},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              if(auto p = m_processRun->getWidget("QuestStateBoard")){
                  p->flipShow();
              }

              m_buttonQuest.stopBlink();
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_right,
      }

    , m_buttonHorse
      {
          DIR_UPLEFT,
          40,
          61,
          {0X0000003C, 0X0000003C, 0X0000003D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              if(auto p = m_processRun->getWidget("HorseBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_right,
      }

    , m_buttonRuntimeConfig
      {
          DIR_UPLEFT,
          72,
          72,
          {0X0000003E, 0X0000003E, 0X0000003F},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              if(auto p = m_processRun->getWidget("RuntimeConfigBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_right,
      }

    , m_buttonFriendChat
      {
          DIR_UPLEFT,
          108,
          61,
          {0X00000040, 0X00000040, 0X00000041},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              if(auto p = m_processRun->getWidget("FriendChatBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_right,
      }

    , m_buttonAC
      {
          DIR_UPLEFT,
          1,
          105,

          proc,
          {
              "AC",
              "MA",
          },

          &m_right,
      }

    , m_buttonDC
      {
          DIR_UPLEFT,
          84,
          105,

          proc,
          {
              "DC",
              "MC",
          },

          &m_right,
      }

    , m_buttonSwitchMode
      {
          DIR_UPLEFT,
          boardW - 178 - 181,
          3,
          {SYS_U32NIL, 0X00000028, 0X00000029},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              switchExpandMode();
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_middle,
      }

    , m_buttonEmoji
      {
          DIR_UPLEFT,
          boardW - 178 - 260,
          87,
          {SYS_U32NIL, 0X00000023, 0X00000024},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_middle,
      }

    , m_buttonMute
      {
          DIR_UPLEFT,
          boardW - 178 - 220,
          87,
          {SYS_U32NIL, 0X00000025, 0X00000026},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_middle,
      }

    , m_levelBox
      {
          DIR_NONE,
          m_middle.w() / 2,
          4,
          proc,

          [this](int dy)
          {
              if(!m_expand){
                  return;
              }

              m_stretchH = std::max<int>(m_stretchH - dy, m_stretchHMin);
              m_stretchH = std::min<int>(m_stretchH, g_sdlDevice->getRendererHeight() - 47 - 55);
              setButtonLoc();
          },
          [this]()
          {
              const int winH = g_sdlDevice->getRendererHeight();
              if(!m_expand){
                  switchExpandMode();
                  m_stretchH = winH - 47 - 55;
                  setButtonLoc();
                  return;
              }

              if(m_stretchH != m_stretchHMin){
                  m_stretchH = m_stretchHMin;
              }
              else{
                  m_stretchH = winH - 47 - 55;
              }
              setButtonLoc();
          },
          &m_middle,
      }

    , m_arcAniBoard
      {
          DIR_UPLEFT,
          (boardW - 178 - 166) / 2 - 18,
         -13,
          0X04000000,
          4,
          1,
          true,
          true,
          &m_middle,
      }

    , m_slider
      {
          DIR_UPLEFT,
          boardW - 178 - 176,
          40,
          5,
          60,

          false,
          2,
          nullptr,

          &m_middle,
      }

    , m_cmdLine
      {
          DIR_UPLEFT,
          7,
          105,
          343 + (boardW - 800),
          17,

          true,

          1,
          12,

          0,
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          [this]()
          {
              inputLineDone();
          },
          nullptr,

          &m_middle,
      }

    , m_logBoard
      {
          DIR_UPLEFT,
          9,
          0, // need reset
          341 + (boardW - 800),

          nullptr,
          0,

          {},
          false,
          false,
          false,
          false,

          1,
          12,
          0,

          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_JUSTIFY,
          0,
          0,

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          nullptr,
          nullptr,

          &m_middle,
      }
{
    if(!proc){
        throw fflerror("invalid ProcessRun provided to ControlBoard()");
    }

    auto fnAssertImage = [](uint32_t img, int w, int h)
    {
        if(auto ptex = g_progUseDB->retrieve(img)){
            int readw = -1;
            int readh = -1;
            if(!SDL_QueryTexture(ptex, 0, 0, &readw, &readh)){
                if(w == readw && h == readh){
                    return;
                }
            }
        }
        throw fflerror("image assertion failed: img = %llu, w = %d, h = %d", to_llu(img), w, h);
    };

    fnAssertImage(0X00000012, 800, 133);
    fnAssertImage(0X00000013, 456, 131);
    fnAssertImage(0X00000022, 127,  41);
    fnAssertImage(0X00000027, 456, 298);

    if(x() != 0 || y() + h() != g_sdlDevice->getRendererHeight() || w() != g_sdlDevice->getRendererWidth()){
        throw fflerror("ControlBoard has wrong location or size");
    }
}

void ControlBoard::update(double fUpdateTime)
{
    m_accuTime += fUpdateTime;
    m_cmdLine.update(fUpdateTime);
    m_logBoard.update(fUpdateTime);
    m_arcAniBoard.update(fUpdateTime);

    m_buttonInventory    .update(fUpdateTime);
    m_buttonHeroState    .update(fUpdateTime);
    m_buttonHeroMagic    .update(fUpdateTime);
    m_buttonGuild        .update(fUpdateTime);
    m_buttonTeam         .update(fUpdateTime);
    m_buttonQuest        .update(fUpdateTime);
    m_buttonHorse        .update(fUpdateTime);
    m_buttonRuntimeConfig.update(fUpdateTime);
    m_buttonFriendChat   .update(fUpdateTime);
}

void ControlBoard::drawLeft() const
{
    const int nY0 = y();

    // draw left part
    if(auto pTexture = g_progUseDB->retrieve(0X00000012)){
        g_sdlDevice->drawTexture(pTexture, 0, nY0, 0, 0, 178, 133);
    }

    // draw HP and MP texture
    {
        auto pHP = g_progUseDB->retrieve(0X00000018);
        auto pMP = g_progUseDB->retrieve(0X00000019);

        if(pHP && pMP){

            // we need to call query
            // so need to validate two textures here

            int nHPH = -1;
            int nHPW = -1;
            int nMPH = -1;
            int nMPW = -1;

            SDL_QueryTexture(pHP, nullptr, nullptr, &nHPW, &nHPH);
            SDL_QueryTexture(pMP, nullptr, nullptr, &nMPW, &nMPH);

            if(auto pMyHero = m_processRun->getMyHero()){
                const double fLostHPRatio = 1.0 - pMyHero->getHealthRatio().at(0);
                const double fLostMPRatio = 1.0 - pMyHero->getHealthRatio().at(1);

                const auto nLostHPH = to_d(std::lround(nHPH * fLostHPRatio));
                const auto nLostMPH = to_d(std::lround(nMPH * fLostMPRatio));

                g_sdlDevice->drawTexture(pHP, 33, nY0 + 8 + nLostHPH, 0, nLostHPH, nHPW, nHPH - nLostHPH);
                g_sdlDevice->drawTexture(pMP, 73, nY0 + 8 + nLostMPH, 0, nLostMPH, nMPW, nMPH - nLostMPH);
            }
        }
    }

    drawHeroLoc();
    m_buttonClose.draw();
    m_buttonMinize.draw();
    m_buttonQuickAccess.draw();

    drawRatioBar(152, nY0 + 115, m_processRun->getMyHero()->getLevelRatio());
    drawRatioBar(165, nY0 + 115, m_processRun->getMyHero()->getInventoryRatio());
}

void ControlBoard::drawRight() const
{
    const int nY0 = y();
    const int nW0 = w();

    // draw right part
    if(auto pTexture = g_progUseDB->retrieve(0X00000012)){
        g_sdlDevice->drawTexture(pTexture, nW0 - 166, nY0, 800 - 166, 0, 166, 133);
    }

    m_buttonExchange.draw();
    m_buttonMiniMap.draw();
    m_buttonMagicKey.draw();

    m_buttonGuild.draw();
    m_buttonTeam.draw();
    m_buttonQuest.draw();
    m_buttonHorse.draw();
    m_buttonRuntimeConfig.draw();
    m_buttonFriendChat.draw();

    m_buttonAC.draw();
    m_buttonDC.draw();

    m_buttonInventory.draw();
    m_buttonHeroState.draw();
    m_buttonHeroMagic.draw();
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

void ControlBoard::drawMiddleDefault() const
{
    const int nY0 = y();
    const int nW0 = w();

    // draw black underlay for the logBoard and actor face
    g_sdlDevice->fillRectangle(colorf::RGBA(0X00, 0X00, 0X00, 0XFF), 178 + 2, nY0 + 14, nW0 - (178 + 2) - (166 + 2), 120);

    m_cmdLine.draw();
    drawLogBoardDefault();
    drawInputGreyBackground();
    drawFocusFace();

    // draw middle part
    if(auto texPtr = g_progUseDB->retrieve(0X00000013)){
        g_sdlDevice->drawTexture(texPtr,             178, nY0 + 2,         0, 0,  50, 131);
        g_sdlDevice->drawTexture(texPtr, nW0 - 166 - 119, nY0 + 2, 456 - 119, 0, 119, 131);

        const int repeatW = 456 - 50 - 119;
        const int drawW   = nW0 - 50 - 119 - 178 - 166;

        const auto [repeat, stretch] = scheduleStretch(drawW, repeatW);
        for(int i = 0; i < repeat; ++i){
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + i * repeatW, nY0 + 2, 50, 0, repeatW, 131);
        }

        // for the rest area
        // need to stretch or shrink
        if(stretch > 0){
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + repeat * repeatW, nY0 + 2, stretch, 131, 50, 0, repeatW, 131);
        }
    }

    // draw title
    // the title texture is not symmetric, add 1 pixel offset
    // then the levelBox can anchor at the middle by m_middle.w() / 2

    if(auto texPtr = g_progUseDB->retrieve(0X00000022)){
        const auto [titleW, titleH] = SDLDeviceHelper::getTextureSize(texPtr);
        const int titleDstX = 178 + (nW0 - 178 - 166 - titleW) / 2 + 1;
        const int titleDstY = nY0 - 19;
        g_sdlDevice->drawTexture(texPtr, titleDstX, titleDstY);
    }

    m_arcAniBoard.draw();
    m_buttonSwitchMode.draw();
    m_levelBox.draw();
    m_slider.draw();
}

void ControlBoard::drawLogBoardDefault() const
{
    const int dstX = 187;
    const int dstY = logBoardStartY();

    const int srcX = 0;
    const int srcY = std::max<int>(0, std::lround((m_logBoard.h() - 83) * m_slider.getValue()));
    const int srcW = m_logBoard.w();
    const int srcH = 83;

    m_logBoard.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}

void ControlBoard::drawLogBoardExpand() const
{
    const int dstX = 187;
    const int dstY = logBoardStartY();

    const int boardFrameH = m_stretchH + 47 + 55 - 70;
    const int srcX = 0;
    const int srcY = std::max<int>(0, std::lround(m_logBoard.h() - boardFrameH) * m_slider.getValue());
    const int srcW = m_logBoard.w();
    const int srcH = boardFrameH;

    m_logBoard.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}

void ControlBoard::drawMiddleExpand() const
{
    const int nY0 = y();
    const int nW0 = w();
    const int nH0 = h();

    // use this position to calculate all points
    // the Y-axis on screen that the big chat-frame starts
    const int startY = nY0 + nH0 - 55 - m_stretchH - 47;

    // draw black underlay for the big log board and input box
    g_sdlDevice->fillRectangle(colorf::RGBA(0X00, 0X00, 0X00, 0XF0), 178 + 2, startY + 2, nW0 - (178 + 2) - (166 + 2), 47 + m_stretchH + 55);

    drawInputGreyBackground();
    m_cmdLine.draw(); // cursor can be over-sized

    if(auto texPtr = g_progUseDB->retrieve(0X00000027)){

        // draw four corners
        g_sdlDevice->drawTexture(texPtr,             178,                   startY,         0,        0,  50, 47);
        g_sdlDevice->drawTexture(texPtr, nW0 - 166 - 119,                   startY, 456 - 119,        0, 119, 47);
        g_sdlDevice->drawTexture(texPtr,             178, startY + 47 + m_stretchH,         0, 298 - 55,  50, 55);
        g_sdlDevice->drawTexture(texPtr, nW0 - 166 - 119, startY + 47 + m_stretchH, 456 - 119, 298 - 55, 119, 55);

        // draw two stretched vertical bars
        const int repeatH = 298 - 47 - 55;
        const auto [repeatHCnt, stretchH] = scheduleStretch(m_stretchH, repeatH);

        for(int i = 0; i < repeatHCnt; ++i){
            g_sdlDevice->drawTexture(texPtr,             178, startY + 47 + i * repeatH,         0, 47,  50, repeatH);
            g_sdlDevice->drawTexture(texPtr, nW0 - 166 - 119, startY + 47 + i * repeatH, 456 - 119, 47, 119, repeatH);
        }

        if(stretchH > 0){
            g_sdlDevice->drawTexture(texPtr,             178, startY + 47 + repeatHCnt * repeatH,  50, stretchH,         0, 47,  50, repeatH);
            g_sdlDevice->drawTexture(texPtr, nW0 - 166 - 119, startY + 47 + repeatHCnt * repeatH, 119, stretchH, 456 - 119, 47, 119, repeatH);
        }

        // draw horizontal top bar and bottom input area
        const int repeatW = 456 - 50 - 119;
        const int drawW   = nW0 - 50 - 119 - 178 - 166;

        const auto [repeatWCnt, stretchW] = scheduleStretch(drawW, repeatW);
        for(int i = 0; i < repeatWCnt; ++i){
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + i * repeatW,                   startY, 50,        0, repeatW, 47);
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + i * repeatW, startY + 47 + m_stretchH, 50, 298 - 55, repeatW, 55);
        }

        if(stretchW > 0){
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + repeatWCnt * repeatW,                   startY, stretchW, 47, 50,        0, repeatW, 47);
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + repeatWCnt * repeatW, startY + 47 + m_stretchH, stretchW, 55, 50, 298 - 55, repeatW, 55);
        }
    }

    if(auto texPtr = g_progUseDB->retrieve(0X00000022)){
        const auto [titleW, titleH] = SDLDeviceHelper::getTextureSize(texPtr);
        const int titleDstX = 178 + (nW0 - 178 - 166 - titleW) / 2 + 1;
        const int titleDstY = startY - 2 - 19;
        g_sdlDevice->drawTexture(texPtr, titleDstX, titleDstY);
    }

    m_arcAniBoard.draw();
    m_buttonSwitchMode.draw();
    m_levelBox.draw();
    m_buttonEmoji.draw();
    m_buttonMute.draw();
    drawLogBoardExpand();
    m_slider.draw();
}

void ControlBoard::drawEx(int, int, int, int, int, int) const
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

    takeEvent |= m_levelBox           .processEvent(event, valid && !takeEvent);
    takeEvent |= m_slider             .processEvent(event, valid && !takeEvent);
    takeEvent |= m_cmdLine            .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonClose        .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonMinize       .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonQuickAccess  .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonExchange     .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonMiniMap      .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonMagicKey     .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonGuild        .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonTeam         .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonQuest        .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonHorse        .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonRuntimeConfig.processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonFriendChat   .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonAC           .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonDC           .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonInventory    .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonHeroState    .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonHeroMagic    .processEvent(event, valid && !takeEvent);
    takeEvent |= m_buttonSwitchMode   .processEvent(event, valid && !takeEvent);

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
                            return valid && m_cmdLine.consumeFocus(true);
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
    m_cmdLine.setFocus(false);

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
                    m_processRun->userCommand(realInput.c_str() + 1);
                }
                break;
            }
        case '$': // lua command for super user
            {
                if(m_processRun){
                    m_processRun->luaCommand(realInput.c_str() + 1);
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

void ControlBoard::addParLog(const char *log)
{
    fflassert(str_haschar(log));
    m_logBoard.addParXML(m_logBoard.parCount(), {0, 0, 0, 0}, log);
    m_slider.setValue(1.0f, false);
}

void ControlBoard::addLog(int logType, const char *log)
{
    if(!log){
        throw fflerror("null log string");
    }

    switch(logType){
        case CBLOG_ERR:
            {
                g_log->addLog(LOGTYPE_WARNING, "%s", log);
                break;
            }
        default:
            {
                g_log->addLog(LOGTYPE_INFO, "%s", log);
                break;
            }
    }

    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    const char *xmlString = [logType]() -> const char *
    {
        // use hex to give alpha
        // color::String2Color has no alpha component

        switch(logType){
            case CBLOG_SYS: return "<par bgcolor = \"rgb(0x00, 0x80, 0x00)\"></par>";
            case CBLOG_DBG: return "<par bgcolor = \"rgb(0x00, 0x00, 0xff)\"></par>";
            case CBLOG_ERR: return "<par bgcolor = \"rgb(0xff, 0x00, 0x00)\"></par>";
            case CBLOG_DEF:
            default       : return "<par></par>";
        }
    }();

    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml template failed: %s", xmlString);
    }

    // to support <, >, / in xml string
    // don't directly pass the raw string to addParXML
    xmlDoc.RootElement()->SetText(log);

    tinyxml2::XMLPrinter printer;
    xmlDoc.Print(&printer);
    m_logBoard.addParXML(m_logBoard.parCount(), {0, 0, 0, 0}, printer.CStr());
    m_slider.setValue(1.0f, false);
}

bool ControlBoard::CheckMyHeroMoved()
{
    return true;
}

void ControlBoard::switchExpandMode()
{
    if(m_expand){
        m_expand = false;
        m_logBoard.setLineWidth(m_logBoard.getLineWidth() - 87);
    }
    else{
        m_expand = true;
        m_stretchH = m_stretchHMin;
        m_logBoard.setLineWidth(m_logBoard.getLineWidth() + 87);
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
        m_levelBox.moveTo((boardW - 178 - 166) / 2, 4 - modeDiffY);
        m_arcAniBoard.moveTo((boardW - 178 - 166 - m_arcAniBoard.w()) / 2, -13 - modeDiffY);

        m_buttonEmoji.moveTo(boardW - 178 - 260, 87);
        m_buttonMute .moveTo(boardW - 178 - 220, 87);

        m_slider.moveTo(w() - 178 - 176, 40 - modeDiffY);
        m_slider.resizeHeight(60 + modeDiffY);
    }
    else{
        m_buttonSwitchMode.moveTo(boardW - 178 - 181, 3);
        m_levelBox.moveTo((boardW - 178 - 166) / 2, 4);
        m_arcAniBoard.moveTo((boardW - 178 - 166 - m_arcAniBoard.w()) / 2, -13);

        m_slider.moveTo(w() - 178 - 176, 40);
        m_slider.resizeHeight(60);
    }
}

int ControlBoard::logBoardStartY() const
{
    if(!m_expand){
        return g_sdlDevice->getRendererHeight() - 120;
    }
    return g_sdlDevice->getRendererHeight() - 55 - m_stretchH - 47 + 12; // 12 is texture top-left to log line distane
}

void ControlBoard::onWindowResize(int winW, int winH)
{
    const auto prevWidth = w();
    m_right.moveBy(winW - w(), 0);
    m_w = winW;

    m_logBoard.setLineWidth(m_logBoard.getLineWidth() + (winW - prevWidth));
    const int maxStretchH = winH - 47 - 55;

    if(m_expand && (m_stretchH > maxStretchH)){
        m_stretchH = maxStretchH;
    }

    m_middle.setW(w() - 178 - 166);
    setButtonLoc();
}

void ControlBoard::drawInputGreyBackground() const
{
    if(!m_cmdLine.focus()){
        return;
    }

    const auto color = colorf::GREY + colorf::A_SHF(48);
    SDLDeviceHelper::EnableRenderBlendMode enableDrawBlendMode(SDL_BLENDMODE_BLEND);

    if(m_expand){

    }
    else{
        g_sdlDevice->fillRectangle(color, m_middle.x() + 7, m_middle.y() + 104, m_middle.w() - 110, 17);
    }
}

void ControlBoard::drawHeroLoc() const
{
    // support different map uses same map name, i.e.
    //
    // 石墓迷宫_1
    // 石墓迷宫_2
    // 石墓迷宫_3
    //
    // we use same map gfx for different map
    // but user still see same map name: 石墓迷宫

    const auto mapName = std::string(to_cstr(DBCOM_MAPRECORD(m_processRun->mapID()).name));
    const auto locStr = str_printf(u8"%s: %d %d", mapName.substr(0, mapName.find('_')).c_str(), m_processRun->getMyHero()->x(), m_processRun->getMyHero()->y());
    LabelBoard locBoard
    {
        DIR_UPLEFT,
        0, // need reset
        0,

        locStr.c_str(),
        1,
        12,
        0,

        colorf::WHITE + colorf::A_SHF(255),
    };

    const int locBoardStartX = (136 - locBoard.w()) / 2;
    const int locBoardStartY = 109;

    locBoard.moveBy(m_left.x() + locBoardStartX, m_left.y() + locBoardStartY);
    locBoard.draw();
}

void ControlBoard::drawRatioBar(int x, int y, double r) const
{
    ImageBoard barImage
    {
        DIR_DOWN,
        0,
        0,

        {},
        {},

        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X000000A0);
        },

        false,
        false,
        0,

        colorf::RGBA(to_u8(255 * r), to_u8(255 * (1 - r)), 0, 255),
    };

    barImage.drawEx(x, y - std::lround(barImage.h() * r), 0, 0, barImage.w(), std::lround(barImage.h() * r));
}

void ControlBoard::drawFocusFace() const
{
    // draw current creature face
    // draw '?' when the face texture is not available

    const auto [faceTexID, hpRatio, buffIDList] = [this]() -> std::tuple<uint32_t, double, const std::optional<SDBuffIDList> &>
    {
        if(const auto coPtr = m_processRun->findUID(m_processRun->getFocusUID(FOCUS_MOUSE))){
            switch(coPtr->type()){
                case UID_PLY:
                    {
                        return
                        {
                            dynamic_cast<Hero *>(coPtr)->faceGfxID(),
                            coPtr->getHealthRatio().at(0),
                            coPtr->getSDBuffIDList(),
                        };
                    }
                case UID_MON:
                    {
                        const auto monFaceTexID = [coPtr]() -> uint32_t
                        {
                            if(const auto lookID = dynamic_cast<ClientMonster*>(coPtr)->lookID(); lookID >= 0){
                                if(const auto texID = to_u32(0X01000000) + (lookID - LID_BEGIN); g_progUseDB->retrieve(texID)){
                                    return texID;
                                }
                            }
                            return 0X010007CF;
                        }();

                        return
                        {
                            monFaceTexID,
                            coPtr->getHealthRatio().at(0),
                            coPtr->getSDBuffIDList(),
                        };
                    }
                default:
                    {
                        break;
                    }
            }
        }

        return
        {
            m_processRun->getMyHero()->faceGfxID(),
            m_processRun->getMyHero()->getHealthRatio().at(0),
            m_processRun->getMyHero()->getSDBuffIDList(),
        };
    }();

    const int nY0 = y();
    const int nW0 = w();

    if(auto faceTexPtr = g_progUseDB->retrieve(faceTexID)){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(faceTexPtr);
        g_sdlDevice->drawTexture(faceTexPtr, nW0 - 266, nY0 + 18, 86, 96, 0, 0, texW, texH);

        constexpr int barWidth  = 87;
        constexpr int barHeight =  5;

        if(auto hpBarPtr = g_progUseDB->retrieve(0X00000015)){
            const auto [barTexW, barTexH] = SDLDeviceHelper::getTextureSize(hpBarPtr);
            g_sdlDevice->drawTexture(hpBarPtr, nW0 - 268, nY0 + 15, std::lround(hpRatio * barWidth), barHeight, 0, 0, barTexW, barTexH);
        }
    }

    if(buffIDList.has_value()){
        constexpr int buffIconDrawW = 16;
        constexpr int buffIconDrawH = 16;

        // +---------------+
        // |               |
        // |               |
        // |               |
        // |               |
        // |               |
        // *---------------+
        // ^
        // +--- (buffIconOffStartX, buffIconOffStartY)

        const int buffIconOffStartX = nW0 - 266;
        const int buffIconOffStartY = nY0 +  99;

        for(int drawIconCount = 0; const auto id: buffIDList.value().idList){
            const auto &br = DBCOM_BUFFRECORD(id);
            fflassert(br);

            if(br.icon.gfxID != SYS_U32NIL){
                if(auto iconTexPtr = g_progUseDB->retrieve(br.icon.gfxID)){
                    const int buffIconOffX = buffIconOffStartX + (drawIconCount % 5) * buffIconDrawW;
                    const int buffIconOffY = buffIconOffStartY - (drawIconCount / 5) * buffIconDrawH;

                    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(iconTexPtr);
                    g_sdlDevice->drawTexture(iconTexPtr, buffIconOffX, buffIconOffY, buffIconDrawW, buffIconDrawH, 0, 0, texW, texH);

                    const auto baseColor = [&br]() -> uint32_t
                    {
                        if(br.favor > 0){
                            return colorf::GREEN;
                        }
                        else if(br.favor == 0){
                            return colorf::YELLOW;
                        }
                        else{
                            return colorf::RED;
                        }
                    }();

                    const auto startColor = baseColor | colorf::A_SHF(255);
                    const auto   endColor = baseColor | colorf::A_SHF( 64);

                    const auto edgeGridCount = (buffIconDrawW + buffIconDrawH) * 2 - 4;
                    const auto startLoc = std::lround(edgeGridCount * std::fmod(m_accuTime, 1500.0) / 1500.0);

                    g_sdlDevice->drawBoxFading(startColor, endColor, buffIconOffX, buffIconOffY, buffIconDrawW, buffIconDrawH, startLoc, buffIconDrawW + buffIconDrawH);
                    drawIconCount++;
                }
            }
        }
    }
}

TritexButton *ControlBoard::getButton(const std::string_view &buttonName)
{
    if     (buttonName == "Inventory"    ){ return &m_buttonInventory    ; }
    else if(buttonName == "HeroState"    ){ return &m_buttonHeroState    ; }
    else if(buttonName == "HeroMagic"    ){ return &m_buttonHeroMagic    ; }
    else if(buttonName == "Guild"        ){ return &m_buttonGuild        ; }
    else if(buttonName == "Team"         ){ return &m_buttonTeam         ; }
    else if(buttonName == "Quest"        ){ return &m_buttonQuest        ; }
    else if(buttonName == "Horse"        ){ return &m_buttonHorse        ; }
    else if(buttonName == "RuntimeConfig"){ return &m_buttonRuntimeConfig; }
    else if(buttonName == "FriendChat"   ){ return &m_buttonFriendChat   ; }
    else                                  { return nullptr               ; }
}

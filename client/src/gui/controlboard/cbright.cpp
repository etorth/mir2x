#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "colorf.hpp"
#include "totype.hpp"
#include "progusedb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "controlboard.hpp"
#include "cbright.hpp"

extern ProgUseDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CBRight::CBRight(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        ProcessRun *argProc,
        Widget     *argParent,
        bool        argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = 166,
          .h = 133,

          .attrs
          {
              .inst
              {
                  .show = [](const Widget *self)
                  {
                      return self->hasParent<ControlBoard>()->m_minimize == false;
                  },
              },
          },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)

    , m_bgFull
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000012);
          },
      }}

    , m_bg
      {{
          .getter = &m_bgFull,
          .vr
          {
              [this]{ return m_bgFull.w() - w(); },
              0,

              [this]{ return w(); },
              [this]{ return h(); },
          },
          .parent{this},
      }}

    , m_buttonExchange
      {{
          .x = 4,
          .y = 6,

          .onOffX = 1,
          .onOffY = 1,

          .onRadius = 10,

          .modColor  = colorf::WHITE + colorf::A_SHF(80),
          .downTexID = 0X00000042U,

          .onTrigger = [this](Widget *, int)
          {
              if(auto cb = hasParent<ControlBoard>()){
                  cb->addLog(0, "exchange doesn't implemented yet");
              }
          },

          .triggerOnDone = true,
          .parent{this},
      }}

    , m_buttonMiniMap
      {{
          .x = 4,
          .y = 40,

          .onOffX = 1,
          .onOffY = 1,

          .onRadius = 10,

          .modColor  = colorf::WHITE + colorf::A_SHF(80),
          .downTexID = 0X00000043U,

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = dynamic_cast<MiniMapBoard *>(m_processRun->getWidget("MiniMapBoard"))){
                  if(p->getMiniMapTexture()){
                      p->flipShow();
                  }
                  else{
                      if(auto cb = hasParent<ControlBoard>()){
                          cb->addLog(CBLOG_ERR, to_cstr(u8"没有可用的地图"));
                      }
                  }
              }
          },

          .triggerOnDone = true,
          .parent{this},
      }}

    , m_buttonMagicKey
      {{
          .x = 4,
          .y = 75,

          .onOffX = 1,
          .onOffY = 1,
          .onRadius = 10,

          .modColor  = colorf::WHITE + colorf::A_SHF(80),
          .downTexID = 0X00000044U,

          .onTrigger = [this](Widget *, int)
          {
              m_processRun->flipDrawMagicKey();
          },

          .triggerOnDone = true,
          .parent{this},
      }}

    , m_buttonInventory
      {{
          .x = 48,
          .y = 33,

          .texAniOpt = 0X00000030,

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("InventoryBoard")){
                  p->flipShow();
              }
          },

          .parent{this},
      }}

    , m_buttonHeroState
      {{
          .x = 77,
          .y = 31,

          .texAniOpt = 0X00000032,

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("MyHeroStateBoard")){
                  p->flipShow();
              }
          },

          .parent{this},
      }}

    , m_buttonHeroMagic
      {{
          .x = 105,
          .y = 33,

          .texAniOpt = 0X00000034,

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("SkillBoard")){
                  p->flipShow();
              }
          },

          .parent{this},
      }}

    , m_buttonGuild
      {{
          .x = 40,
          .y = 11,

          .texAniOpt = 0X00000036,

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("GuildBoard")){
                  p->flipShow();
              }
          },

          .parent{this},
      }}

    , m_buttonTeam
      {{
          .x = 72,
          .y = 8,

          .texAniOpt = 0X00000038,

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("TeamStateBoard")){
                  p->flipShow();
              }
          },

          .parent{this},
      }}

    , m_buttonQuest
      {{
          .x = 108,
          .y = 11,

          .texAniOpt = 0X0000003A,

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("QuestStateBoard")){
                  p->flipShow();
              }

              m_buttonQuest.stopBlink();
          },

          .parent{this},
      }}

    , m_buttonHorse
      {{
          .x = 40,
          .y = 61,

          .texAniOpt = 0X0000003C,

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("HorseBoard")){
                  p->flipShow();
              }
          },

          .parent{this},
      }}

    , m_buttonRuntimeConfig
      {{
          .x = 72,
          .y = 72,

          .texAniOpt = 0X0000003E,

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("RuntimeConfigBoard")){
                  p->flipShow();
              }
          },

          .parent{this},
      }}

    , m_buttonFriendChat
      {{
          .x = 108,
          .y = 61,

          .texAniOpt = 0X00000040,

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("FriendChatBoard")){
                  p->flipShow();
              }
          },

          .parent{this},
      }}

    , m_buttonAC
      {{
          .x = 1,
          .y = 105,

          .proc = argProc,
          .names
          {
              "AC",
              "MA",
          },

          .parent{this},
      }}

    , m_buttonDC
      {{
          .x = 84,
          .y = 105,

          .proc = argProc,
          .names
          {
              "DC",
              "MC",
          },

          .parent{this},
      }}
{}

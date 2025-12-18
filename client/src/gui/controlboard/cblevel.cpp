#include "cblevel.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"

extern SDLDevice *g_sdlDevice;

CBLevel::CBLevel(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        ProcessRun *argProc,
        CBLevel::TriggerCBFunc argOnClick,

        Widget *argParent,
        bool    argAutoDelete)

    : ButtonBase
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = 16,
          .h = 16,

          .onTrigger = std::move(argOnClick),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_image
      {{
          .dir = DIR_NONE,

          .x = [this](const Widget *){ return w() / 2; },
          .y = [this](const Widget *){ return h() / 2; },

          .texLoadFunc = [](const Widget *)
          {
              return g_sdlDevice->getCover(8, 360);
          },

          .modColor = [this](const Widget *) -> uint32_t
          {
              switch(getState()){
                  case BEVENT_ON  : return colorf::BLUE + colorf::A_SHF(0XFF);
                  case BEVENT_DOWN: return colorf::RED  + colorf::A_SHF(0XFF);
                  default         : return 0;
              }
          },

          .parent{this},
      }}

    , m_level
      {{
          .dir = DIR_NONE,

          .x = [this](const Widget *){ return w() / 2; },
          .y = [this](const Widget *){ return h() / 2; },

          .textFunc = [this](const Widget *) -> std::string
          {
              return std::to_string(m_processRun->getMyHero()->getLevel());
          },

          .font
          {
              .id = 0,
              .size = 12,
              .color = colorf::YELLOW + colorf::A_SHF(255),
          },

          .parent{this},
      }}
{}

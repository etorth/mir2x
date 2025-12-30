#include "acbutton.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ACButton::ACButton(ACButton::InitArgs args)
    : TrigfxButton
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .onTrigger = [this](int)
          {
              m_buttonIndex = (m_buttonIndex + 1) % m_buttonNameList.size();
          },

          .onClickDone = false,
          .parent = std::move(args.parent),
      }}

    , m_proc(fflcheck(args.proc))
    , m_texMap
      {
          {"AC", 0X00000046},
          {"DC", 0X00000047},
          {"MA", 0X00000048},
          {"MC", 0X00000049},
      }

    , m_buttonNameList(std::move(fflcheck(args.names, !args.names.empty())))
    , m_img
      {{
          .texLoadFunc = [this]{ return g_progUseDB->retrieve(m_texMap.at(buttonName())); },
          .modColor = [this] -> uint32_t
          {
              if(getState() == BEVENT_OFF) return colorf::WHITE_A255;
              else                         return colorf::  RED_A255;
          },
      }}

    , m_text
      {{
          .textFunc = [this]
          {
              const auto [low, high] = m_proc->getACNum(buttonName());
              return str_printf("%d-%d", low, high);
          },

          .font
          {
              .color = [this] -> uint32_t
              {
                  if(getState() == BEVENT_OFF) return colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF);
                  else                         return colorf::RGBA(0XFF, 0X00, 0X00, 0XFF);
              },
          },
      }}

    , m_gfxCanvas
      {{
          .flex = [this]{ return m_img.w() + 5 + m_text.w(); },

          .v = false,
          .align = ItemAlign::CENTER,

          .first{&m_img},
          .second{&m_text},
      }}
{
    setGfxFunc([this]{ return &m_gfxCanvas; });
}

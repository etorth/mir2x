#include "widget.hpp"
#include "cbtitle.hpp"
#include "progusedb.hpp"
#include "controlboard.hpp"

extern ProgUseDB *g_progUseDB;

CBTitle::CBTitle(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_bg
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(0X00000022); },
          .parent{this},
      }}

    , m_arcAni
      {{
          .x = 46,
          .y = 8,

          .texAniID = 0X04000000,
          .fps = 1,
          .crossFade = true,

          .parent{this},
      }}

    , m_level
      {
          DIR_NONE,
          63,
          25,

          argProc,
          [this](Widget *self, int) // double-click
          {
              auto &hide = self->hasParent<ControlBoard>()->m_minimize; hide = !hide;
          },

          this,
          false,
      }
{
    setSize([this]{ return m_bg.w(); },
            [this]{ return m_bg.h(); });
}

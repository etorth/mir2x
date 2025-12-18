#include "widget.hpp"
#include "cbtitle.hpp"
#include "pngtexdb.hpp"
#include "controlboard.hpp"

extern PNGTexDB *g_progUseDB;

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
      {
          DIR_UPLEFT,
          46,
          8,

          0X04000000,
          4,
          1,

          true,
          true,

          this,
          false,
      }

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

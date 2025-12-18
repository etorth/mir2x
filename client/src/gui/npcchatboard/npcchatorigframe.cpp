#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "npcchatorigframe.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

NPCChatOrigFrame::NPCChatOrigFrame(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .w = std::nullopt,
          .h = std::nullopt,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_up
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000051); },
          .parent{this},
      }}

    , m_down
      {{
          .x = 0,
          .y = [this]{ return m_up.h(); },

          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000053); },
          .parent{this},
      }}
{}

#include "npcchatframe.hpp"

NPCChatFrame::NPCChatFrame(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarSizeOpt argW,
        Widget::VarSizeOpt argH,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .w = std::move(argW),
          .h = std::move(argH),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_frame{}
    , m_board
      {{
          .getter = &m_frame,
          .vr
          {
              40,
              50,

              300,
              110,
          },

          .resize
          {
              [this]{ return w() - (m_frame.w() - 300); },
              [this]{ return h() - (m_frame.h() - 110); },
          },

          .parent{this},
      }}
{}

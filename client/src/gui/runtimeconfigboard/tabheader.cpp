#include "tabheader.hpp"

TabHeader::TabHeader(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabel,
        std::function<void(Widget *, int)> argOnClick,

        std::any argData,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,
          .w = {},
          .h = {},

          .attrs
          {
              .inst
              {
                  .data = std::move(argData),
              }
          },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_label
      {{
          .label = argLabel,
          .font
          {
              .id = 1,
              .size = 14,
          },
      }}

    , m_button
      {{
          .gfxList
          {
              &m_label,
              &m_label,
              &m_label,
          },

          .onTrigger = std::move(argOnClick),

          .offXOnClick = 1,
          .offYOnClick = 1,

          .parent{this},
      }}
{}

#include "fontselector.hpp"

FontSelector::FontSelector(FontSelector::InitArgs args)
    : Widget
      {{
      }}

    , m_widget
      {{
          .label
          {
              .text = u8"控件",
          },

          .title
          {
              .text = u8"选择控件",
          },

          .itemList
          {
              {
                  .gfxWidget
                  {
                      .widget = new TextBoard{{}},
                      .autoDelete = true,
                  },
              },

              {
                  .gfxWidget
                  {
                      .widget = new TextBoard{{}},
                      .autoDelete = true,
                  },
              },
          },
      }}

    , m_widget
      {{
          .x = [this]{ return m_widget.w(); },

          .label
          {
              .text = u8"字体",
          },

          .title
          {
              .text = u8"选择字体",
          },
      }}

    , m_size
      {{
          .x = [this]{ return m_font.dx() + m_font.w(); },
          .parent{this},
      }}
{
}

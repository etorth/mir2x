#include "labelsliderbar.hpp"

LabelSliderBar::LabelSliderBar(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabel,
        int argLabelWidth,

        int argSliderIndex,
        int argSliderWidth,
        std::function<void(float)> argOnValueChange,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_label
      {{
          .label = argLabel,
      }}

    , m_labelCrop
      {{
          .getter = &m_label,
          .vr
          {
              fflcheck(argLabelWidth, argLabelWidth >= 0),
              m_label.h(),
          },
          .parent{this},
      }}

    , m_slider
      {{
          .bar
          {
              .w = fflcheck(argSliderWidth, argSliderWidth >= 0),
              .h = TexSliderBar::BAR_FIXED_EDGE_SIZE,
              .v = false,
          },

          .index = argSliderIndex,
          .onChange = std::move(argOnValueChange),

          .parent{this},
      }}
{
    setW(m_labelCrop.w() + m_slider.w());
    setH(std::max<int>({m_labelCrop.h(), m_slider.h()}));

    m_labelCrop.moveAt(DIR_LEFT, 0                                 , h() / 2);
    m_slider   .moveAt(DIR_LEFT, m_labelCrop.dx() + m_labelCrop.w(), h() / 2);
}

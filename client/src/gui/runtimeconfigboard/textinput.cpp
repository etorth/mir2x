#include "textinput.hpp"
#include "pngtexdb.hpp"

extern PNGTexDB *g_progUseDB;

TextInput::TextInput(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabelFirst,
        const char8_t *argLabelSecond,

        int argGapFirst,
        int argGapSecond,

        bool argIMEEnabled,

        int argInputW,
        int argInputH,

        std::function<void()> argOnTab,
        std::function<void()> argOnCR,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,
          .x = argX,
          .y = argY,
          .w = {},
          .h = {},
          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_labelFirst
      {{
          .label = argLabelFirst,
          .font
          {
              .id = 1,
              .size = 12,
          },
          .parent{this},
      }}

    , m_labelSecond
      {{
          .label = argLabelSecond,
          .font
          {
              .id = 1,
              .size = 12,
          },
          .parent{this},
      }}

    , m_image
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(0X00000460); },
      }}

    , m_imageBg
      {{
          .getter = &m_image,
          .vr
          {
              3,
              3,
              m_image.w() - 6,
              2,
          },

          .resize
          {
              argInputW,
              argInputH,
          },

          .parent{this},
      }}

    , m_input
      {
          DIR_UPLEFT,
          0,
          0,

          argInputW,
          argInputH,

          argIMEEnabled,

          1,
          12,
          0,
          colorf::WHITE_A255,

          2,
          colorf::WHITE_A255,

          argOnTab,
          argOnCR,
          nullptr,

          this,
          false,
      }
{
    const int maxH = std::max<int>({m_labelFirst.h(), m_imageBg.h(), m_labelSecond.h()});

    m_labelFirst .moveAt(DIR_LEFT,                                                   0, maxH / 2);
    m_imageBg    .moveAt(DIR_LEFT, m_labelFirst.dx() + m_labelFirst.w() + argGapFirst , maxH / 2);
    m_labelSecond.moveAt(DIR_LEFT, m_imageBg   .dx() + m_imageBg   .w() + argGapSecond, maxH / 2);

    m_input.moveAt(DIR_UPLEFT, m_imageBg.dx() + 3, m_imageBg.dy() + 2);
}

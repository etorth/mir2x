#include "strf.hpp"
#include "textinput.hpp"

TextInput::TextInput(TextInput::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .parent = std::move(args.parent),
      }}

    , m_labelFirst(str_haschar(args.labelFirst) ? new LabelBoard
      {{
          .dir = DIR_LEFT,

          .x = 0,
          .y = [this]{ return h() / 2; },

          .label = args.labelFirst,
          .font = args.font, // no move
          .parent
          {
              .widget = this,
              .autoDelete = true,
          },
      }} : nullptr)

    , m_bg
      {{
          .dir = DIR_LEFT,

          .x = [firstGap = std::move(args.gapFirst), this]
          {
              if(m_labelFirst){
                  return m_labelFirst->dx() + m_labelFirst->w() + Widget::evalSize(firstGap, this);
              }
              else{
                  return 0;
              }
          },
          .y = [this]{ return h() / 2; },

          .v = false,
          .parent{this},
      }}

    , m_labelSecond(str_haschar(args.labelSecond) ? new LabelBoard
      {{
          .dir = DIR_LEFT,

          .x = [secondGap = std::move(args.gapSecond), this]
          {
              return m_bg.dx() + m_bg.w() + Widget::evalSize(secondGap, this);
          },
          .y = [this]{ return h() / 2; },

          .label = args.labelSecond,
          .font = args.font,
          .parent
          {
              .widget = this,
              .autoDelete = true,
          },
      }} : nullptr)

    , m_input
      {{
          .x = [this]{ return m_bg.dx() + m_bg.getInputROI().x; },
          .y = [this]{ return m_bg.dy() + m_bg.getInputROI().y; },
          .w = [this]{ return             m_bg.getInputROI().w; },
          .h = [this]{ return             m_bg.getInputROI().h; },

          .enableIME = std::move(args.enableIME),
          .font = std::move(args.font),

          .onTab = std::move(args.onTab),
          .onCR  = std::move(args.onCR),

          .parent{this},
      }}
{
    m_bg.setInputSize(std::move(args.inputSize));
    setSize([this]
    {
        if(m_labelSecond){
            return m_labelSecond->dx() + m_labelSecond->w();
        }
        else{
            return m_bg.dx() + m_bg.w();
        }
    },

    [this]
    {
        return std::max<int>(
        {
            m_labelFirst  ? m_labelFirst ->h() : 0,
            m_labelSecond ? m_labelSecond->h() : 0,

            m_bg.h(),
            m_input.h(), // no need since it's inside m_bg
        });
    });
}

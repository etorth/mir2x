#include <utility>
#include "textbutton.hpp"

TextButton::TextButton(TextButton::InitArgs args)
    : TrigfxButton
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .seff = std::move(args.seff),

          .onOverIn  = std::move(args.onOverIn),
          .onOverOut = std::move(args.onOverOut),

          .onClick = std::move(args.onClick),
          .onTrigger = std::move(args.onTrigger),

          .offXOnOver = args.offXOnOver,
          .offYOnOver = args.offYOnOver,

          .offXOnClick = args.offXOnClick,
          .offYOnClick = args.offYOnClick,

          .onClickDone = args.onClickDone,
          .radioMode   = args.radioMode,

          .attrs = std::move(args.attrs),
          .parent = std::move(args.parent),
      }}

    , m_text
      {{
          .textFunc = std::move(args.textFunc),
          .font     = std::move(args.font    ),
      }}
{
    m_text.setFontColor([this]
    {
        switch(getState()){
            case BEVENT_OFF : return colorf:: YELLOW_A255;
            case BEVENT_ON  : return colorf::  GREEN_A255;
            case BEVENT_DOWN: return colorf::MAGENTA_A255;
            default: throw fflpanic("invalid button state: {}", getState());
        }
    });
    setGfxFunc([this]{ return &m_text; });
}

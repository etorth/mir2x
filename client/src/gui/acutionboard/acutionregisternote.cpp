#include <utility>
#include "colorf.hpp"
#include "acutionregisternote.hpp"

AcutionRegisterNote::AcutionRegisterNote(AcutionRegisterNote::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = 222,
          .h = 116,

          .attrs
          {
              .inst = std::move(args.attrs),
          },
          .parent = std::move(args.parent),
      }}

    , m_note
      {{
          .lineWidth = 222,
          .canEdit = true,

          .enableIME = std::move(args.enableIME),
          .font
          {
              .color = colorf::WHITE_A255,
          },

          .lineAlign = LALIGN_LEFT,
          .parent{this},
      }}

    , m_hint
      {{
          .x = 2,
          .y = 2,
          .textFunc = to_cstr(u8"可填写联系方式或其它说明"),
          .font
          {
              .color = colorf::RGBA(220, 220, 220, 128),
          },
          .attrs
          {
              .show = [this]
              {
                  return !m_note.hasToken() && !m_note.focus();
              },
          },
          .parent{this},
      }}
{}

bool AcutionRegisterNote::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(Widget::processEventDefault(event, valid && m_enabled, m)){
        return true;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) && (event.button.button == SDL_BUTTON_LEFT) && m.in(to_d(event.button.x), to_d(event.button.y))){
        if(m_enabled){
            m_note.setFocus(true);
        }
        return consumeFocus(true);
    }
    return consumeFocus(false);
}

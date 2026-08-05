#include <utility>
#include "strf.hpp"
#include "xmlf.hpp"
#include "colorf.hpp"
#include "acutionitemdetailupper.hpp"

AcutionItemDetailUpper::AcutionItemDetailUpper(AcutionItemDetailUpper::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = 228,
          .h = 145,

          .parent = std::move(args.parent),
      }}

    , m_onContactSeller(std::move(args.onContactSeller))

    , m_title
      {{
          .dir = DIR_NONE,
          .x = 76,
          .y = 10,
          .textFunc = [this] -> const char *
          {
              return m_titleString.empty() ? to_cstr(u8"卖家留言") : m_titleString.c_str();
          },

          .font
          {
              .color = colorf::CYAN_A255,
          },
          .parent{this},
      }}

    , m_noteArea
      {{
          .x = 7,
          .y = 54,
          .w = 211,
          .h = 52,
          .parent{this},
      }}

    , m_note
      {{
          .lineWidth = 211,
          .lineAlign = LALIGN_JUSTIFY,
          .parent{&m_noteArea},
      }}

    , m_buttonContactSeller
      {{
          .dir = DIR_NONE,
          .x = 186,
          .y = 10,
          .textFunc = to_cstr(u8"发消息"),
          .onTrigger = [this](Widget *, int)
          {
              if(!m_seller.empty() && m_onContactSeller){
                  m_onContactSeller(m_seller);
              }
          },

          .attrs
          {
              .active = [this]{ return !m_seller.empty() && bool(m_onContactSeller); },
          },
          .parent{this},
      }}
{
    setShow(false);
}

void AcutionItemDetailUpper::setItem(const SDAcutionItem *item)
{
    if(item){
        const auto notePar = xmlf::toParString("%s", item->note.empty() ? to_cstr(u8"卖家未填写留言。") :  item->note.c_str());
        const auto noteXML = str_printf("<layout>%s</layout>", notePar.c_str());

        m_note.loadXML(noteXML.c_str());
        m_seller = item->seller;
        m_titleString = str_printf("卖家：%s", to_cstr(item->seller.name));
    }
    else{
        m_note.clear();
        m_seller.clear();
        m_titleString.clear();
    }

    flipShow(item);
}

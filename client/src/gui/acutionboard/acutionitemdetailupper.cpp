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

          .w = 226,
          .h = 115,

          .parent = std::move(args.parent),
      }}

    , m_title
      {{
          .dir = DIR_LEFT,
          .x = 7,
          .y = 8,
          .textFunc = to_cstr(u8"卖家留言"),
          .font
          {
              .color = colorf::CYAN_A255,
          },
          .parent{this},
      }}

    , m_noteArea
      {{
          .x = 7,
          .y = 27,
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

    , m_sellerArea
      {{
          .x = 7,
          .y = 92,
          .w = 155,
          .h = 15,
          .parent{this},
      }}

    , m_seller
      {{
          .lineWidth = 155,
          .lineAlign = LALIGN_LEFT,
          .parent{&m_sellerArea},
      }}

    , m_buttonContactSeller
      {{
          .dir = DIR_UPRIGHT,
          .x = 218,
          .y = 91,
          .textFunc = to_cstr(u8"联系卖家"),
          .onTrigger = [](Widget *, int)
          {
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

        const auto sellerPar = xmlf::toParString("卖家：%s", to_cstr(item->seller));
        const auto sellerXML = str_printf("<layout>%s</layout>", sellerPar.c_str());
        m_seller.loadXML(sellerXML.c_str());
    }
    else{
        m_note.clear();
        m_seller.clear();
    }

    flipShow(item);
}

#include <algorithm>
#include <cmath>
#include <utility>
#include "colorf.hpp"
#include "dbcomid.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "acutionregisterbox.hpp"

extern PNGTexDB *g_itemDB;

AcutionRegisterBox::AcutionRegisterBox(AcutionRegisterBox::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = 159,
          .h = 164,
          .attrs
          {
              .inst = std::move(args.attrs),
          },
          .parent = std::move(args.parent),
      }}

    , m_onClick(std::move(args.onClick))

    , m_image
      {{
          .x = [this]{ return (159 - itemImageSize().first) / 2; },
          .y = [this]{ return 5 + (124 - itemImageSize().second) / 2; },
          .w = [this]{ return itemImageSize().first; },
          .h = [this]{ return itemImageSize().second; },
          .texLoadFunc = [this]{ return itemTexture(); },
          .parent{this},
      }}

    , m_name
      {{
          .dir = DIR_NONE,
          .x = 159 / 2,
          .y = 164 - 18,
          .textFunc = [this]() -> std::string
          {
              if(m_item){
                  return to_cstr(DBCOM_ITEMRECORD(m_item.itemID).name);
              }
              return {};
          },
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}
{}

bool AcutionRegisterBox::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
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

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN
            && event.button.button == SDL_BUTTON_LEFT
            && m.in(to_d(event.button.x), to_d(event.button.y))){
        if(m_enabled && m_onClick){
            m_onClick();
        }
        return true;
    }
    return consumeFocus(false);
}

SDL_Texture *AcutionRegisterBox::itemTexture() const
{
    if(!m_item){
        return nullptr;
    }

    const auto &ir = DBCOM_ITEMRECORD(m_item.itemID);
    fflassert(ir);

    if(auto texPtr = g_itemDB->retrieve(ir.pkgGfxID | 0X02000000)){
        return texPtr;
    }

    if(false
            || ir.wearable(WLG_DRESS)
            || ir.wearable(WLG_HELMET)
            || ir.wearable(WLG_WEAPON)
            || ir.wearable(WLG_SHOES)
            || ir.wearable(WLG_NECKLACE)
            || ir.wearable(WLG_ARMRING0)
            || ir.wearable(WLG_RING0)
            || ir.wearable(WLG_TORCH)){
        return g_itemDB->retrieve(ir.pkgGfxID | 0X01000000);
    }
    return nullptr;
}

std::pair<int, int> AcutionRegisterBox::itemImageSize() const
{
    constexpr int maxItemW = 159 - 20;
    constexpr int maxItemH = 164 - 40;

    if(auto texPtr = itemTexture()){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        const auto ratio = std::max<double>({to_df(texW) / maxItemW, to_df(texH) / maxItemH, 1.0});
        return
        {
            to_d(std::lround(texW / ratio)),
            to_d(std::lround(texH / ratio)),
        };
    }
    return {0, 0};
}

#include <algorithm>
#include <utility>
#include "strf.hpp"
#include "colorf.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "acutionregisterboard.hpp"
#include "acutionboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern Client *g_client;

AcutionBoard::AcutionBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .dir = DIR_NONE,
          .x = []{ return g_sdlDevice->getRendererWidth () / 2; },
          .y = []{ return g_sdlDevice->getRendererHeight() / 2; },
          .w = std::nullopt,
          .h = std::nullopt,
          .parent{argParent, argAutoDelete},
      }}

    , m_runProc(fflcheck(argProc))
    , m_background
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00001400); },
          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_itemList
      {{
          .x = 10,
          .y = 84,
          .parent{this},
      }}

    , m_category
      {{
          .dir = DIR_NONE,
          .x = 61,
          .y = 20,
          .textFunc = [this]() -> std::string
          {
              switch(m_itemList.getItemList().category){
                  case ACUTIONCAT_ALL     : return to_cstr(u8"所有物品");
                  case ACUTIONCAT_DRESS   : return to_cstr(u8"衣服");
                  case ACUTIONCAT_WEAPON  : return to_cstr(u8"武器");
                  case ACUTIONCAT_NECKLACE: return to_cstr(u8"项链");
                  case ACUTIONCAT_HELMET  : return to_cstr(u8"头盔");
                  case ACUTIONCAT_RING    : return to_cstr(u8"戒指");
                  case ACUTIONCAT_ARMRING : return to_cstr(u8"手镯");
                  case ACUTIONCAT_SHOES   : return to_cstr(u8"鞋类");
                  case ACUTIONCAT_POTION  : return to_cstr(u8"药品");
                  case ACUTIONCAT_BOOK    : return to_cstr(u8"图书");
                  case ACUTIONCAT_OTHER   : return to_cstr(u8"其他物品");
                  default                 : return to_cstr(u8"寄售");
              }
          },
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_itemCount
      {{
          .dir = DIR_NONE,
          .x = 240,
          .y = 20,
          .textFunc = [this]() -> std::string
          {
              return to_cstr(str_printf(u8"共%zu件", m_itemList.size()));
          },
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_itemRange
      {{
          .dir = DIR_NONE,
          .x = 419,
          .y = 20,
          .textFunc = [this]() -> std::string
          {
              if(const auto firstIndex = m_itemList.firstIndex(); firstIndex.has_value()){
                  const auto lastIndex = std::min(firstIndex.value() + AcutionItemList::m_rowCount, m_itemList.size());
                  return to_cstr(str_printf(u8"第%zu-%zu件物品", firstIndex.value() + 1, lastIndex));
              }
              return {};
          },
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },
          .attrs
          {
              .show = [this]{ return !m_itemList.empty(); },
          },
          .parent{this},
      }}

    , m_columnItem
      {{
          .dir = DIR_NONE,
          .x = 64,
          .y = 62,
          .textFunc = to_cstr(u8"物品"),
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_columnSeller
      {{
          .dir = DIR_NONE,
          .x = 188,
          .y = 62,
          .textFunc = to_cstr(u8"寄售人"),
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_columnTime
      {{
          .dir = DIR_NONE,
          .x = 293,
          .y = 62,
          .textFunc = to_cstr(u8"剩余"),
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_columnPrice
      {{
          .dir = DIR_NONE,
          .x = 399,
          .y = 62,
          .textFunc = to_cstr(u8"价格"),
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_itemDetailUpper
      {{
          .x = 486,
          .y = 50,
          .parent{this},
      }}

    , m_itemDetailLower
      {{
          .x = 486,
          .y = 168,
          .parent{this},
      }}

    , m_buttonPrevious
      {{
          .x = 21,
          .y = 360,
          .texIDList
          {
              .off  = 0X00000311,
              .on   = 0X00000310,
              .down = 0X00000312,
          },
          .onTrigger = [this](Widget *, int)
          {
              m_itemList.movePrev();
              updateSelectedItem();
          },

          .attrs
          {
              .active = [this]{ return m_itemList.canMovePrev(); },
          },
          .parent{this},
      }}

    , m_buttonNext
      {{
          .x = 72,
          .y = 360,
          .texIDList
          {
              .off  = 0X00000301,
              .on   = 0X00000300,
              .down = 0X00000302,
          },
          .onTrigger = [this](Widget *, int)
          {
              m_itemList.moveNext();
              updateSelectedItem();
          },

          .attrs
          {
              .active = [this]{ return m_itemList.canMoveNext(); },
          },
          .parent{this},
      }}

    , m_buttonRefresh
      {{
          .x = 122,
          .y = 361,
          .texIDList
          {
              .off  = 0X00000331,
              .on   = 0X00000330,
              .down = 0X00000332,
          },
          .onTrigger = [this](Widget *, int)
          {
              const auto  currCategory = m_itemList.getItemList().category;
              const auto queryCategory = (currCategory >= ACUTIONCAT_BEGIN && currCategory < ACUTIONCAT_END) ? currCategory : ACUTIONCAT_ALL;

              g_client->send({CM_QUERYACUTIONITEMLIST, CMQueryAcutionItemList
              {
                  .category = to_u8(queryCategory),
              }});
          },
          .parent{this},
      }}

    , m_buttonItemSearch
      {{
          .x = 412,
          .y = 360,
          .texIDList
          {
              .on   = 0X00001420,
              .down = 0X00001421,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonSellerSearch
      {{
          .x = 412,
          .y = 390,
          .texIDList
          {
              .on   = 0X00001420,
              .down = 0X00001421,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonRegister
      {{
          .x = 513,
          .y = 370,
          .texIDList
          {
              // looks there are two version of register button, in GameInter
              //
              // TMP001228.PNG vs TMP001904.PNG
              // they are identical expect 1 pixel offset
              //
              // zsdb uses 1228, but acution board frame image uses 1904
              // here use zsdb and off texture to redraw the button off state

              .off  = 0X000000B3,
              .on   = 0X000000B3,
              .down = 0X000000B4,
          },
          .onTrigger = [this](Widget *, int)
          {
              if(auto registerBoardPtr = dynamic_cast<AcutionRegisterBoard *>(m_runProc->getWidget("AcutionRegisterBoard"))){
                  registerBoardPtr->beginRegister();
              }
          },
          .parent{this},
      }}

    , m_buttonBuy
      {{
          .x = 563,
          .y = 370,
          .texIDList
          {
              .on   = 0X000000B7,
              .down = 0X000000B8,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonCancel
      {{
          .x = 643,
          .y = 370,
          .texIDList
          {
              .off  = 0X00000850, // same as buttonRegister
              .on   = 0X00000850,
              .down = 0X00000851,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = 680,
          .y = 396,
          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },
          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
          },
          .parent{this},
      }}
{
    setShow(false);
}

bool AcutionBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    const auto oldSelectedIndex = m_itemList.selectedIndex();
    if(m_itemList.processEventParent(event, valid, m)){
        if(oldSelectedIndex != m_itemList.selectedIndex()){
            updateSelectedItem();
        }
        return true;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_itemDetailUpper.show() && m_itemDetailUpper.processEventParent(event, valid, m)){ return true; }
    if(m_buttonPrevious    .processEventParent(event, valid, m)){ return true; }
    if(m_buttonNext        .processEventParent(event, valid, m)){ return true; }
    if(m_buttonRefresh     .processEventParent(event, valid, m)){ return true; }
    if(m_buttonItemSearch  .processEventParent(event, valid, m)){ return true; }
    if(m_buttonSellerSearch.processEventParent(event, valid, m)){ return true; }
    if(m_buttonRegister    .processEventParent(event, valid, m)){ return true; }
    if(m_buttonBuy         .processEventParent(event, valid, m)){ return true; }
    if(m_buttonCancel      .processEventParent(event, valid, m)){ return true; }
    if(m_buttonClose       .processEventParent(event, valid, m)){ return true; }

    switch(event.type){
        case SDL_EVENT_KEY_DOWN:
            {
                if(event.key.key == SDLK_ESCAPE){
                    setShow(false);
                    setFocus(false);
                    return true;
                }
                return consumeFocus(false);
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                return consumeFocus(m.in(to_d(event.button.x), to_d(event.button.y)));
            }
        case SDL_EVENT_MOUSE_MOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(to_d(event.motion.x), to_d(event.motion.y)) || focus())){
                    const int remapX = m.x - m.ro->x;
                    const int remapY = m.y - m.ro->y;
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();

                    const int newX = std::clamp(remapX + to_d(event.motion.xrel), 0, rendererW - w());
                    const int newY = std::clamp(remapY + to_d(event.motion.yrel), 0, rendererH - h());
                    moveBy(newX - remapX, newY - remapY);
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void AcutionBoard::setItemList(SDAcutionItemList sdAcutionItemList)
{
    m_itemList.setItemList(std::move(sdAcutionItemList));
    updateSelectedItem();
}

void AcutionBoard::updateSelectedItem()
{
    const SDAcutionItem *item = nullptr;
    if(const auto selectedIndex = m_itemList.selectedIndex(); selectedIndex.has_value()){
        item = fflcheck(m_itemList.indexItem(selectedIndex.value()));
    }

    m_itemDetailUpper.setItem(item);
    m_itemDetailLower.setItem(item);
}

#include <limits>
#include <utility>
#include <algorithm>
#include <charconv>
#include "xmlf.hpp"
#include "colorf.hpp"
#include "dbcomid.hpp"
#include "layoutboard.hpp"
#include "progusedb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"
#include "inputstringboard.hpp"
#include "stallboard.hpp"

extern ProgUseDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

StallBoard::StallBoard(StallBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .parent = std::move(args.parent),
      }}

    , m_runProc(fflcheck(args.runProc))

    , m_background
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000230); },
          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_sellerName
      {{
          .dir = DIR_NONE,
          .x = 49,
          .y = 26,

          .textFunc = [this]{ return m_runProc->getStallSellerName(m_stall.dbid); },
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::WHITE_A255,
          },

          .parent{this},
      }}

    , m_stallName
      {{
          .dir = DIR_NONE,
          .x = 157,
          .y = 26,

          .textFunc = [this]{ return m_stall.name; },
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },

          .parent{this},
      }}

    , m_grid
      {{
          .x = 17,
          .y = 58,
          .parent{this},
      }}

    , m_infoLine0
      {{
          .dir = DIR_LEFT,
          .x = 36,
          .y = 332,

          .textFunc = [this] -> std::string
          {
              if(!isSeller() && !m_stall.selling){
                  return to_cstr(u8"摊位已关闭");
              }

              if(m_grid.selectedItem()){
                  return str_printf(to_cstr(u8"单价：%s"), str_ksep(m_grid.selectedPrice()).c_str());
              }

              return str_printf(to_cstr(u8"物品：%zu/%zu"), m_stall.itemList.size(), SYS_STALLMAXITEM);
          },

          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::WHITE_A255,
          },

          .parent{this},
      }}

    , m_infoLine1
      {{
          .dir = DIR_LEFT,
          .x = 36,
          .y = 354,

          .textFunc = [this] -> std::string
          {
              if(isSeller()){
                  size_t total = 0;
                  for(const auto &stallItem: m_stall.itemList){
                      total += stallItem.price * stallItem.item.count;
                  }
                  return str_printf(to_cstr(u8"总值：%s"), str_ksep(total).c_str());
              }
              return str_printf(to_cstr(u8"金币：%s"), str_ksep(m_runProc->getMyHero()->getGold()).c_str());
          },

          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },

          .parent{this},
      }}

    , m_opBackground
      {{
          .x = 196,
          .y = 324,

          .texLoadFunc = []{ return g_progUseDB->retrieve(0X000000B0); },
          .parent{this},
      }}

    , m_buttonItemOp
      {{
          .x = 199,
          .y = 327,

          .texAniOpt = 0X000000B7,
          .onTrigger = [this](Widget *, int){ onItemOp(); },

          .attrs
          {
              .show = [this]{ return showItemOp(); },
          },

          .parent{this},
      }}

    , m_buttonStartStall
      {{
          .x = 199,
          .y = 327,

          .texAniOpt = 0X00000910,
          .onTrigger = [this](Widget *, int){ onStartStall(); },

          .attrs
          {
              .show = [this]{ return showStartStall(); },
          },

          .parent{this},
      }}

    , m_buttonCloseStall
      {{
          .x = 199,
          .y = 327,

          .texAniOpt = 0X00000860,
          .onTrigger = [this](Widget *, int){ onCloseStall(); },

          .attrs
          {
              .show = [this]{ return showCloseStall(); },
          },

          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = 240,
          .y = 345,

          .texAniOpt = 0X0000001C,
          .onTrigger = [this](Widget *, int){ onClose(); },

          .parent{this},
      }}
{
    m_opBackground.setShow([this]
    {
        return showItemOp() || showStartStall() || showCloseStall();
    });

    setShow(false);
    setSize([this]{ return m_background.w(); },
            [this]{ return m_background.h(); });
}

void StallBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    Widget::drawDefault(m);

    if(const auto item = m_grid.hoveredItem()){
        drawItemHoverText(*item, m_grid.hoveredPrice());
    }
}

bool StallBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE){
        onClose();
        return true;
    }

    if(Widget::processEventDefault(event, valid, m)){
        return true;
    }

    switch(event.type){
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                m_dragging = event.button.button == SDL_BUTTON_LEFT && m.in(to_d(event.button.x), to_d(event.button.y));
                return consumeFocus(m_dragging);
            }
        case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                return std::exchange(m_dragging, false) ? consumeFocus(m.in(to_d(event.button.x), to_d(event.button.y))) : false;
            }
        case SDL_EVENT_MOUSE_MOTION:
            {
                if(m_dragging && (event.motion.state & SDL_BUTTON_LMASK)){
                    const int remapX = m.x - m.ro->x;
                    const int remapY = m.y - m.ro->y;
                    const auto [winLogicalW, winLogicalH] = g_sdlDevice->getWindowLogicalSize();

                    const int newX = std::clamp(remapX + to_d(event.motion.xrel), 0, winLogicalW - w());
                    const int newY = std::clamp(remapY + to_d(event.motion.yrel), 0, winLogicalH - h());
                    moveBy(newX - remapX, newY - remapY);
                    return consumeFocus(true);
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}

void StallBoard::loadStall(SDStall sdS)
{
    if(sdS.dbid != m_stall.dbid){
        m_grid.clearSelection();
    }

    m_stall = std::move(sdS);
    m_grid.setItemList(m_stall.itemList);
}

bool StallBoard::isSeller() const
{
    return m_stall.dbid && (m_stall.dbid == m_runProc->getMyHeroDBID());
}

bool StallBoard::showItemOp() const
{
    return m_grid.selectedItem() && (isSeller() || m_stall.selling);
}

bool StallBoard::showStartStall() const
{
    return isSeller() && !m_stall.selling && !m_stall.itemList.empty() && !m_grid.selectedItem();
}

bool StallBoard::showCloseStall() const
{
    return isSeller() && m_stall.selling && !m_grid.selectedItem();
}

void StallBoard::onItemOp()
{
    const auto item = m_grid.selectedItem();
    if(!item){
        return;
    }

    if(isSeller()){
        removeItem(*item);
    }
    else{
        buyItem(*item, m_grid.selectedPrice(), item->count);
    }
}

void StallBoard::onStartStall()
{
    auto inputBoard = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    inputBoard->waitInput(
    {
        .layoutString = u8"<layout><par>请输入摊位名称</par></layout>",
        .onAccept = [this](std::u8string input)
        {
            const std::string name(reinterpret_cast<const char *>(input.data()), input.size());
            if(name.empty() || name.size() >= SYS_STALLNAMESIZE || !utf8f::valid(name)){
                m_runProc->addCBLog(CBLOG_ERR, u8"无效的摊位名称");
                return;
            }
            m_runProc->requestStartStall(name);
        },

        .validate = [](const std::string &currentInput, const std::string &newInput)
        {
            return currentInput.size() + newInput.size() < SYS_STALLNAMESIZE;
        },
    });
}

void StallBoard::onCloseStall()
{
    auto inputBoard = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    inputBoard->waitChoice(u8"<layout><par>你确定要收摊吗？</par></layout>", false, [this]
    {
        m_runProc->requestCloseStall();
        setShow(false);
    });
}

void StallBoard::onClose()
{
    if(isSeller()){
        if(!m_stall.selling && !m_stall.itemList.empty()){
            // cancel stall setup, all items go back to inventory
            m_runProc->requestCloseStall();
        }

        if(auto invBoard = dynamic_cast<InventoryBoard *>(m_runProc->getWidget("InventoryBoard")); invBoard->getInvOp() == INVOP_STALL){
            invBoard->clearInvOp();
        }
    }

    m_dragging = false;
    m_grid.clearSelection();
    setShow(false);
}

void StallBoard::removeItem(const SDItem &item)
{
    if(!m_stall.selling){
        m_runProc->requestRemoveStallItem(item.itemID, item.seqID);
        return;
    }

    auto inputBoard = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    const auto prompt = to_u8rawstr("<layout>" + xmlf::toParString("你确定要下架%s吗？", to_cstr(DBCOM_ITEMRECORD(item.itemID).name)) + "</layout>");
    inputBoard->waitChoice(prompt, false, [itemID = item.itemID, seqID = item.seqID, this]
    {
        m_runProc->requestRemoveStallItem(itemID, seqID);
    });
}

void StallBoard::buyItem(const SDItem &item, size_t price, size_t maxCount)
{
    if(!m_stall.selling){
        return;
    }

    auto inputBoard = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    const auto fnBuy = [sellerDBID = m_stall.dbid, itemID = item.itemID, seqID = item.seqID, price, this](size_t count)
    {
        if(price * count > m_runProc->getMyHero()->getGold()){
            m_runProc->addCBLog(CBLOG_ERR, u8"金币不足");
            return;
        }
        m_runProc->requestBuyStallItem(sellerDBID, itemID, seqID, count, price);
    };

    const std::string itemName = to_cstr(DBCOM_ITEMRECORD(item.itemID).name);
    if(maxCount <= 1){
        const auto prompt = to_u8rawstr("<layout>" + xmlf::toParString("你确定要花费%s金币购买%s吗？", str_ksep(price).c_str(), itemName.c_str()) + "</layout>");
        inputBoard->waitChoice(prompt, false, [fnBuy]
        {
            fnBuy(1);
        });
        return;
    }

    // count input works as confirmation, unit price is shown in the prompt
    inputBoard->waitInput(
    {
        .layoutString = to_u8rawstr("<layout>" + xmlf::toParString("%s单价%s金币，请输入购买数量（1-%zu）", itemName.c_str(), str_ksep(price).c_str(), maxCount) + "</layout>"),
        .onAccept = [fnBuy, maxCount, this](std::u8string input)
        {
            const std::string value(reinterpret_cast<const char *>(input.data()), input.size());
            size_t count = 0;
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), count, 10);
            if(value.empty()
                    || ec != std::errc()
                    || ptr != value.data() + value.size()
                    || count == 0
                    || count > maxCount){
                m_runProc->addCBLog(CBLOG_ERR, u8"无效的购买数量：%s", value.c_str());
                return;
            }
            fnBuy(count);
        },

        .validate = [](const std::string &currentInput, const std::string &newInput)
        {
            return currentInput.size() + newInput.size() <= 10
                && newInput.find_first_not_of("0123456789") == std::string::npos;
        },
    });
}

void StallBoard::drawItemHoverText(const SDItem &item, size_t price) const
{
    const LayoutBoard hoverTextBoard
    {{
        .lineWidth = 200,
        .initXML = to_cstr(item.getXMLLayout(
        {
            {SDItem::XML_PRICE, str_ksep(price)},
            {SDItem::XML_PRICECOLOR, "green"},
        }).c_str()),
        .lineAlign = LALIGN_JUSTIFY,
    }};

    const auto [mousePX, mousePY] = g_sdlDevice->getMousePLoc();
    const int textBoxW = std::max<int>(hoverTextBoard.w(), 200) + 20;
    const int textBoxH = hoverTextBoard.h() + 20;

    const int drawBoardPX = mathf::bound<int>(mousePX, 0, g_sdlDevice->getWindowLogicalWidth () - textBoxW);
    const int drawBoardPY = mathf::bound<int>(mousePY, 0, g_sdlDevice->getWindowLogicalHeight() - textBoxH);

    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0, 0, 200), drawBoardPX, drawBoardPY, textBoxW, textBoxH, 5);
    g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 200), drawBoardPX, drawBoardPY, textBoxW, textBoxH, 5);
    hoverTextBoard.draw({.x = drawBoardPX + 10, .y = drawBoardPY + 10});
}

#include "colorf.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "dropitemrulerow.hpp"
#include "runtimeconfigboard.hpp"

extern SDLDevice *g_sdlDevice;
DropItemRuleRow::DropItemRuleRow(RuntimeConfigBoard *configBoard, uint32_t itemID)
    : Widget
      {{
          .w = 410,
          .h = 22,
      }}

    , m_configBoard(configBoard)
    , m_itemID(itemID)

    , m_bg
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
          {
              const auto rule = m_configBoard->dropItemRule(m_itemID);
              if     (rule & DIRF_HIGHLIGHT){ g_sdlDevice->fillRectangle(colorf::RGBA(255, 255, 0, 45), drawDstX, drawDstY, w(), h(), 3); }
              else if(rule & DIRF_FILTER   ){ g_sdlDevice->fillRectangle(colorf::RGBA(255,   0, 0, 35), drawDstX, drawDstY, w(), h(), 3); }
          },
          .parent{this},
      }}

    , m_name
      {{
          .x = 0,
          .y = 3,
          .label = DBCOM_ITEMRECORD(itemID).name,

          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::WHITE_A255,
          },
          .parent{this},
      }}

    , m_highlight
      {{
          .x = 240,
          .y = 2,
          .gap = 3,
          .box{.w = 14, .h = 14},

          .label
          {
              .text = u8"高亮",
              .font{.id = 1, .size = 12},
          },

          .getter = [this]{ return (m_configBoard->dropItemRule(m_itemID) & DIRF_HIGHLIGHT) != 0; },
          .setter = [this](bool value){ m_configBoard->setDropItemRule(m_itemID, DIRF_HIGHLIGHT, value); },
          .parent{this},
      }}

    , m_filter
      {{
          .x = 304,
          .y = 2,
          .gap = 3,
          .box
          {
              .w = 14,
              .h = 14,
              .color = colorf::RGBA(231, 120, 120, 160),
          },

          .label
          {
              .text = u8"隐藏",
              .font{.id = 1, .size = 12},
          },

          .getter = [this]{ return (m_configBoard->dropItemRule(m_itemID) & DIRF_FILTER) != 0; },
          .setter = [this](bool value){ m_configBoard->setDropItemRule(m_itemID, DIRF_FILTER, value); },
          .parent{this},
      }}
{
    fflassert(m_configBoard);
    fflassert(DBCOM_ITEMRECORD(m_itemID), m_itemID);
}

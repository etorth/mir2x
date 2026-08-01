#include "colorf.hpp"
#include "dbcomid.hpp"
#include "itembox.hpp"
#include "checklabel.hpp"
#include "labelboard.hpp"
#include "sdldevice.hpp"
#include "dropitemrulerow.hpp"
#include "runtimeconfigboard.hpp"

extern SDLDevice *g_sdlDevice;
DropItemRuleRow::DropItemRuleRow(Widget::VarSize argW, RuntimeConfigBoard *configBoard, uint32_t itemID)
    : Widget
      {{
          .w = std::move(argW),
          .h = 22,
      }}

    , m_configBoard(fflcheck(configBoard))
    , m_itemID(fflcheck(itemID, DBCOM_ITEMRECORD(itemID)))

    , m_bg
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
          {
              const auto rule = m_configBoard->dropItemRule(m_itemID);
              if     (rule & DIRF_HIGHLIGHT){ g_sdlDevice->fillRectangle(colorf::RGBA(255, 255, 0, 45), drawDstX, drawDstY, w(), h()); }
              else if(rule & DIRF_FILTER   ){ g_sdlDevice->fillRectangle(colorf::RGBA(255,   0, 0, 35), drawDstX, drawDstY, w(), h()); }
          },
          .parent{this},
      }}

    , m_pair
      {{
          .dir = DIR_LEFT,

          .y    = [this]{ return h() / 2; },
          .flex = [this]{ return w()    ; },

          .v = false,
          .align = ItemAlign::CENTER,

          .first
          {
              .widget = new LabelBoard{{.label = DBCOM_ITEMRECORD(itemID).name}},
              .autoDelete = true,
          },

          .second
          {
              .widget = new ItemBox
              {{
                  .v = false,
                  .align = ItemAlign::CENTER,

                  .itemSpace = 10,
                  .tailSpace = 10,

                  .childList
                  {
                      {
                          new CheckLabel
                          {{
                              .gap = 3,
                              .box{.w = 14, .h = 14},
                              .label{.text = u8"高亮"},

                              .getter = [this]{ return (m_configBoard->dropItemRule(m_itemID) & DIRF_HIGHLIGHT) != 0; },
                              .setter = [this](bool value){ m_configBoard->setDropItemRule(m_itemID, DIRF_HIGHLIGHT, value); },
                          }},
                          true,
                      },
                      {
                          new CheckLabel
                          {{
                              .gap = 3,
                              .box{.w = 14, .h = 14, .color = colorf::RGBA(231, 120, 120, 160)},
                              .label{.text = u8"隐藏"},

                              .getter = [this]{ return (m_configBoard->dropItemRule(m_itemID) & DIRF_FILTER) != 0; },
                              .setter = [this](bool value){ m_configBoard->setDropItemRule(m_itemID, DIRF_FILTER, value); },
                          }},
                          true,
                      },
                  },
              }},
              .autoDelete = true,
          },

          .parent{this},
      }}
{}

#include <algorithm>
#include "totype.hpp"
#include "colorf.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "dropitemrulerow.hpp"
#include "dropitemruleboard.hpp"
#include "runtimeconfigboard.hpp"

extern SDLDevice *g_sdlDevice;
DropItemRuleBoard::DropItemRuleBoard(RuntimeConfigBoard *configBoard)
    : Widget
      {{
          .w = 450,
          .h = 330,
      }}

    , m_configBoard(configBoard)

    , m_itemBox
      {{
          .fixed = m_listW - 8,
          .v = true,

          .headSpace = 4,
          .itemSpace = 2,
          .tailSpace = 4,
      }}

    , m_scroll
      {{
          .x = m_listX,
          .y = m_listY,

          .vpw = m_listW,
          .vph = m_listH,

          .getter = std::addressof(m_itemBox),

          .hScroll = false,  // ItemBox is fixed-width and never overflows horizontally

          .fgDrawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), drawDstX, drawDstY, self->w(), self->h());
          },

          .parent{this},
      }}
{
    fflassert(m_configBoard);
    for(uint32_t itemID = 1; itemID < DBCOM_ITEMENDID(); ++itemID){
        if(const auto &ir = DBCOM_ITEMRECORD(itemID); ir && ir.pkgGfxID >= 0){
            m_itemBox.addItem(new DropItemRuleRow(m_configBoard, itemID), true);
        }
    }
}

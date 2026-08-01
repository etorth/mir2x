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
          .w = std::nullopt,
          .h = std::nullopt,
      }}

    , m_configBoard(fflcheck(configBoard))
    , m_itemBox
      {{
          .fixed = m_viewportW,
          .v = true,

          .headSpace = 4,
          .itemSpace = 2,
          .tailSpace = 4,
      }}

    , m_scroll
      {{
          .vpw = m_viewportW,
          .vph = m_viewportH,

          .getter = std::addressof(m_itemBox),
          .fgDrawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), drawDstX, drawDstY, self->w(), self->h());
          },

          .parent{this},
      }}
{
    for(uint32_t itemID = 1; itemID < DBCOM_ITEMENDID(); ++itemID){
        if(const auto &ir = DBCOM_ITEMRECORD(itemID); ir && ir.pkgGfxID >= 0){
            m_itemBox.addItem(new DropItemRuleRow(m_viewportW, m_configBoard, itemID), true);
        }
    }
}

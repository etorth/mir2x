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
    , m_listBg
      {{
          .x = m_listX,
          .y = m_listY,
          .w = m_listW,
          .h = m_listH,

          .drawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::RGBA(  0,   0,   0,  60), drawDstX, drawDstY, self->w(), self->h(), 6);
              g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), drawDstX, drawDstY, self->w(), self->h(), 6);
          },

          .parent{this},
      }}

    , m_itemBox
      {{
          .fixed = m_listW - 8,
          .v = true,

          .headSpace = 4,
          .itemSpace = 2,
          .tailSpace = 4,
      }}

    , m_viewport
      {{
          .x = m_listX,
          .y = m_listY,
          .getter = std::addressof(m_itemBox),
          .vr = Widget::VarROI(0, [this]{ return to_dround(scrollMax() * m_slider.getValue()); }, m_listW, m_listH),
          .parent{this},
      }}

    , m_slider
      {{
          .bar
          {
              .x = m_sliderX,
              .y = m_listY,
              .w = TexSliderBar::BAR_FIXED_EDGE_SIZE,
              .h = m_listH,
              .v = true,
          },

          .index = 3,
          .parent{this},
      }}
{
    fflassert(m_configBoard);
    for(uint32_t itemID = 1; itemID < DBCOM_ITEMENDID(); ++itemID){
        if(const auto &ir = DBCOM_ITEMRECORD(itemID); ir && ir.pkgGfxID >= 0){
            m_itemBox.addItem(new DropItemRuleRow(m_configBoard, itemID), true);
        }
    }

    if(scrollMax() <= 0){
        m_slider.setShow(false);
    }
}

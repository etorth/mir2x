#include <utility>
#include "strf.hpp"
#include "totype.hpp"
#include "colorf.hpp"
#include "itembox.hpp"
#include "labelboard.hpp"
#include "sdldevice.hpp"
#include "trigfxbutton.hpp"
#include "rankingrow.hpp"

namespace
{
    std::u8string getRankingLabel(size_t rank, RankingType type, const SDRankingEntry &entry)
    {
        const auto name = to_u8rawstr(entry.name);
        switch(type){
            case RANKING_LEVEL:
                return str_printf(u8"第%zu名  %s  [DBID:%u]  等级:%u", rank, name.c_str(), entry.dbid, entry.level);
            case RANKING_GOLD:
                {
                    const auto gold = to_u8rawstr(str_ksep(entry.gold));
                    return str_printf(u8"第%zu名  %s  [DBID:%u]  金币:%s", rank, name.c_str(), entry.dbid, gold.c_str());
                }
            default:
                throw fflpanic("invalid ranking type: {}", to_d(type));
        }
    }

    class RankingActionButton final: public Widget
    {
        private:
            LabelBoard m_label;
            TrigfxButton m_button;

        public:
            RankingActionButton(const char8_t *label, uint32_t dbid)
                : Widget
                  {{
                      .w = std::nullopt,
                      .h = std::nullopt,

                      .attrs
                      {
                          .inst
                          {
                              .data = dbid,
                          },
                      },
                  }}

                , m_label
                  {{
                      .label = label,
                      .font
                      {
                          .id = 1,
                          .size = 12,
                          .color = colorf::YELLOW_A255,
                      },
                  }}

                , m_button
                  {{
                      .gfxList
                      {
                          &m_label,
                          &m_label,
                          &m_label,
                      },

                      .onTrigger = [](Widget *, int)
                      {
                      },

                      .offXOnClick = 1,
                      .offYOnClick = 1,

                      .parent{this},
                  }}
            {}
    };
}

extern SDLDevice *g_sdlDevice;

RankingRow::RankingRow(Widget::VarSize argW, size_t rank, RankingType type, const SDRankingEntry &entry)
    : Widget
      {{
          .w = std::move(argW),
          .h = 24,
      }}

    , m_dbid(entry.dbid)
    , m_bg
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [rank](const Widget *self, int drawDstX, int drawDstY)
          {
              if(rank % 2 == 0){
                  g_sdlDevice->fillRectangle(colorf::RGBA(255, 255, 255, 20), drawDstX, drawDstY, self->w(), self->h());
              }
          },

          .parent{this},
      }}

    , m_pair
      {{
          .dir = DIR_LEFT,

          .y = [this]{ return h() / 2; },
          .flex = [this]{ return w(); },

          .v = false,
          .align = ItemAlign::CENTER,

          .first
          {
              .widget = new LabelBoard
              {{
                  .label = getRankingLabel(rank, type, entry).c_str(),
                  .font
                  {
                      .id = 1,
                      .size = 12,
                  },
              }},
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
                      {new RankingActionButton(u8"打招呼", m_dbid), true},
                      {new RankingActionButton(u8"加好友", m_dbid), true},
                  },
              }},
              .autoDelete = true,
          },

          .parent{this},
      }}
{}

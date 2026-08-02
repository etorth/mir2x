#include <format>
#include <utility>
#include "strf.hpp"
#include "totype.hpp"
#include "colorf.hpp"
#include "itembox.hpp"
#include "sdldevice.hpp"
#include "textboard.hpp"
#include "textbutton.hpp"
#include "rankingrow.hpp"

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

          .y    = [this]{ return h() / 2; },
          .flex = [this]{ return w()    ; },

          .v = false,
          .align = ItemAlign::CENTER,

          .first
          {
              .widget = new TextBoard
              {{
                  .textFunc = [rank, type, entry] -> std::string
                  {
                      switch(type){
                          case RANKING_LEVEL: return std::format("第{}名  {}  [DBID:{}]  等级 {}", rank, entry.name.c_str(), entry.dbid, entry.level);
                          case RANKING_GOLD : return std::format("第{}名  {}  [DBID:{}]  金币 {}", rank, entry.name.c_str(), entry.dbid, str_ksep(entry.gold));
                          default: throw fflpanic("invalid ranking type: {}", to_d(type));
                      }
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
                      {
                          new TextButton
                          {{
                              .textFunc = to_cstr(u8"打招呼"),
                              .onTrigger = [](Widget *, int)
                              {
                              },

                              .attrs
                              {
                                  .data = m_dbid,
                              },
                          }},
                          true,
                      },
                      {
                          new TextButton
                          {{
                              .textFunc = to_cstr(u8"加好友"),
                              .onTrigger = [](Widget *, int)
                              {
                              },

                              .attrs
                              {
                                  .data = m_dbid,
                              },
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

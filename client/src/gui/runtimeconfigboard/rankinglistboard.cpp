#include <memory>
#include "totype.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "rankingrow.hpp"
#include "rankinglistboard.hpp"

extern SDLDevice *g_sdlDevice;

RankingListBoard::RankingListBoard(RankingType type)
    : Widget
      {{
          .w = std::nullopt,
          .h = std::nullopt,
      }}

    , m_type(type)
    , m_itemBox
      {{
          .fixed = m_viewportW,
      }}

    , m_scroll
      {{
          .vpw = m_viewportW,
          .vph = m_viewportH,

          .getter = std::addressof(m_itemBox),

          .hBar    = false,
          .hScroll = false,
          .fgDrawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), drawDstX, drawDstY, self->w(), self->h());
          },

          .parent{this},
      }}
{
    fflassert(type == RANKING_LEVEL || type == RANKING_GOLD, to_d(type));
}

void RankingListBoard::setRankingList(const SDRankingList &rankingList)
{
    fflassert(rankingList.type == m_type, rankingList.type, to_d(m_type));

    m_itemBox.clearItem();
    for(size_t i = 0; i < rankingList.entries.size(); ++i){
        m_itemBox.addItem(new RankingRow(m_viewportW, i + 1, m_type, rankingList.entries.at(i)), true);
    }
    m_scroll.scrollTo(0, 0);
}

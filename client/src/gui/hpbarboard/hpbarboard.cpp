#include <algorithm>
#include <cmath>
#include "hpbarboard.hpp"
#include "processrun.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "gfxshapeboard.hpp"
#include "dbcomid.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

namespace
{
    constexpr int buffIconDrawW = 10;
    constexpr int buffIconDrawH = 10;
    constexpr int buffIconsPerRow = 3;
    constexpr int buffIconInsetX = 1;
    constexpr double buffIconAnimationPeriod = 1500.0;

    class HPBuffIcon: public Widget
    {
        private:
            ImageBoard m_image;
            GfxShapeBoard m_border;

        public:
            HPBuffIcon(Widget *parent, uint32_t gfxID, int favor, const double &animationTime)
                : Widget
                  {{
                      .x = [](const Widget *widget)
                      {
                          return to_d(widget->childID().value() % buffIconsPerRow) * buffIconDrawW;
                      },
                      .y = [parent](const Widget *widget)
                      {
                          const auto lastRow = parent->lastChild()->childID().value() / buffIconsPerRow;
                          const auto row = widget->childID().value() / buffIconsPerRow;
                          return to_d(lastRow - row) * buffIconDrawH;
                      },
                      .w = buffIconDrawW,
                      .h = buffIconDrawH,
                      .parent{parent, true},
                  }}
                , m_image
                  {{
                      .w = buffIconDrawW,
                      .h = buffIconDrawH,
                      .texLoadFunc = [gfxID]{ return g_progUseDB->retrieve(gfxID); },
                      .parent{this},
                  }}
                , m_border
                  {{
                      .w = buffIconDrawW,
                      .h = buffIconDrawH,
                      .drawFunc = [favor, &animationTime](const Widget *, int x, int y)
                      {
                          const auto color = favor > 0 ? colorf::GREEN : favor == 0 ? colorf::YELLOW : colorf::RED;
                          constexpr int edgeGridCount = (buffIconDrawW + buffIconDrawH) * 2 - 4;
                          const auto startLoc = std::lround(edgeGridCount * std::fmod(animationTime, buffIconAnimationPeriod) / buffIconAnimationPeriod);
                          g_sdlDevice->drawBoxFading(color | colorf::A_SHF(255), color | colorf::A_SHF(64), x, y, buffIconDrawW, buffIconDrawH, startLoc, buffIconDrawW + buffIconDrawH);
                      },
                      .parent{this},
                  }}
            {}
    };
}

HPBarBoard::HPBarBoard(ProcessRun *processRun, const std::optional<SDHealth> &health, const double &animationTime)
    : Widget
      {{
          .w = [this]{ return barWidth(); },
          .h = [this]{ return m_iconArea.h() + barHeight(); },
      }}
    , m_processRun(processRun)
    , m_health(health)
    , m_animationTime(animationTime)
    , m_bar
      {{
          .y = [this]{ return m_iconArea.h(); },
          .w = [this]{ return barWidth(); },
          .h = [this]{ return barHeight(); },
          .attrs{.inst{.show = [this]{ return showBar(); }}},
          .parent{this},
      }}
    , m_fillClip
      {{
          .w = [this]{ return to_d(std::lround(barWidth() * healthRatio())); },
          .h = [this]{ return barHeight(); },
          .parent{&m_bar},
      }}
    , m_fill
      {{
          .texLoadFunc = [this]{ return g_progUseDB->retrieve(numericHealth() ? 0X00000017 : 0X00000015); },
          .parent{&m_fillClip},
      }}
    , m_frame
      {{
          .texLoadFunc = [this]{ return g_progUseDB->retrieve(numericHealth() ? 0X00000016 : 0X00000014); },
          .parent{&m_bar},
      }}
    , m_numbers
      {{
          .x = [this]{ return (barWidth() - m_numbers.w()) / 2; },
          .y = [this]{ return (barHeight() - m_numbers.h()) / 2; },
          .textFunc = [this]
          {
              const int hp = m_health.has_value() ? m_health->hp : 0;
              const int maxHP = m_health.has_value() ? m_health->getMaxHP() : 0;
              return str_printf("%*d/%d", std::max<int>(1, to_d(std::to_string(maxHP).size())), hp, maxHP);
          },
          .font{.id = 15, .size = 16, .color = colorf::WHITE_A255},
          .attrs{.show = [this]{ return numericHealth() && m_health.has_value(); }},
          .parent{&m_bar},
      }}
    , m_iconArea
      {{
          .x = [this]
          {
              const int baseBarW = SDLDeviceHelper::getTextureWidth(g_progUseDB->retrieve(0X00000014));
              return buffIconInsetX - (baseBarW - barWidth()) / 2;
          },
          .w = std::nullopt,
          .h = std::nullopt,
          .parent{this},
      }}
{
    fflassert(m_processRun);
}

bool HPBarBoard::numericHealth() const
{
    return m_processRun->getRuntimeConfig<RTCFG_数字显血>();
}

bool HPBarBoard::showBar() const
{
    return m_processRun->getRuntimeConfig<RTCFG_显示血条>()
        && (!m_processRun->getRuntimeConfig<RTCFG_满血不显血>()
            || (m_health.has_value() && m_health->hp < m_health->getMaxHP()));
}

int HPBarBoard::barWidth() const
{
    return SDLDeviceHelper::getTextureWidth(m_frame.getTexture());
}

int HPBarBoard::barHeight() const
{
    return SDLDeviceHelper::getTextureHeight(m_frame.getTexture());
}

double HPBarBoard::healthRatio() const
{
    return m_health.has_value() && m_health->getMaxHP() > 0
        ? std::clamp(to_df(m_health->hp) / m_health->getMaxHP(), 0.0, 1.0)
        : 1.0;
}

void HPBarBoard::setBuffIDList(const SDBuffIDList &list)
{
    m_iconArea.clearChild();
    m_iconArea.purge();
    for(const auto id: list.idList){
        const auto &br = DBCOM_BUFFRECORD(id);
        fflassert(br);
        if(br.icon.gfxID != SYS_U32NIL && g_progUseDB->retrieve(br.icon.gfxID)){
            new HPBuffIcon(&m_iconArea, br.icon.gfxID, br.favor, m_animationTime);
        }
    }
}

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

class HPBuffIcon: public Widget
{
    public:
        constexpr static int buffIconDrawW = 10;
        constexpr static int buffIconDrawH = 10;

        constexpr static int buffIconsPerRow = 3; // when HP numbers disabled
        constexpr static int buffIconInsetX  = 1;

        constexpr static double buffIconAniPeriod = 1500.0;

    private:
        struct InitArgs
        {
            uint32_t gfxID = SYS_U32NIL;
            int      favor = 0;

            std::function<int()> rowIcons {};
            Widget::WADPair parent {};
        };

    private:
        ImageBoard m_image;
        GfxShapeBoard m_border;

    private:
        double m_accuTime = 0;

    public:
        HPBuffIcon(HPBuffIcon::InitArgs args)
            : Widget
              {{
                  .x = [rowIcons = args.rowIcons](const Widget *widget)
                  {
                      return to_d(widget->childID().value() % rowIcons()) * buffIconDrawW;
                  },

                  .y = [parent = args.parent.widget, rowIcons = args.rowIcons](const Widget *widget)
                  {
                      const auto  iconsPerRow = rowIcons();
                      const auto     childRow = static_cast<int>(widget             ->childID().value()) / iconsPerRow;
                      const auto lastChildRow = static_cast<int>(parent->lastChild()->childID().value()) / iconsPerRow;

                      return (lastChildRow - childRow) * buffIconDrawH;
                  },

                  .w = buffIconDrawW,
                  .h = buffIconDrawH,

                  .parent = std::move(args.parent),
              }}

            , m_image
              {{
                  .w = buffIconDrawW,
                  .h = buffIconDrawH,

                  .texLoadFunc = [gfxID = args.gfxID]{ return g_progUseDB->retrieve(gfxID); },
                  .parent{this},
              }}

            , m_border
              {{
                  .w = buffIconDrawW,
                  .h = buffIconDrawH,

                  .drawFunc = [favor = args.favor, this](const Widget *, int x, int y)
                  {
                      const auto color = [favor]
                      {
                          if     (favor < 0){ return colorf::RED   ; }
                          else if(favor > 0){ return colorf::GREEN ; }
                          else              { return colorf::YELLOW; }
                      }();

                      const auto edgePixelCount = (buffIconDrawW + buffIconDrawH) * 2 - 4;
                      const auto startLoc = static_cast<int>(edgePixelCount * std::fmod(m_accuTime, buffIconAniPeriod) / buffIconAniPeriod);

                      g_sdlDevice->drawBoxFading(
                              color + colorf::A_SHF(255),
                              color + colorf::A_SHF( 64),

                              x,
                              y,

                              buffIconDrawW,
                              buffIconDrawH,

                              startLoc,
                              buffIconDrawW + buffIconDrawH);
                  },
                  .parent{this},
              }}
        {}

    public:
        void updateDefault(double fUpdateTime) override
        {
            m_accuTime += fUpdateTime;
        }
};

HPBarBoard::HPBarBoard(HPBarBoard::InitArgs args)
    : Widget
      {{
          .w = [this]{ return std::max<int>(baseBarWidth(), barWidth()); },
          .h = [this]{ return              m_iconArea.h() + barHeight(); },
      }}

    , m_runProc(fflcheck(args.runProc))
    , m_creature(fflcheck(args.creature))

    , m_bar
      {{
          .dir = DIR_UP,
          .x = [this]{ return w() / 2; },
          .y = [this]{ return m_iconArea.h(); },

          .w = [this]{ return barWidth (); },
          .h = [this]{ return barHeight(); },

          .attrs
          {
              .inst
              {
                  .show = [this]{ return showBar(); },
              },
          },

          .parent{this},
      }}

    , m_fillClip
      {{
          .w = [this]{ return barWidthClip(); },
          .h = [this]{ return barHeight   (); },

          .parent{&m_bar},
      }}

    , m_fill
      {{
          .w = [this]{ return barWidth(); },
          .texLoadFunc = [this]{ return g_progUseDB->retrieve(showNumber() ? 0X00000017 : 0X00000015); },
          .parent{&m_fillClip},
      }}

    , m_frame
      {{
          .texLoadFunc = [this]{ return g_progUseDB->retrieve(showNumber() ? 0X00000016 : 0X00000014); },
      }}

    , m_resizedFrame
      {{
          .getter = &m_frame,
          .vr
          {
              BAR_W_MARGIN,
              0,
              [this]{ return m_frame.w() - 2 * BAR_W_MARGIN; },
              [this]{ return m_frame.h(); },
          },

          .resize
          {
              [this]{ return barWidth() - 2 * BAR_W_MARGIN; },
              [this]{ return m_frame.h(); },
          },

          .parent{&m_bar},
      }}

    , m_numbers
      {{
          .dir = DIR_NONE,

          .x = [this]{ return m_bar.w() / 2; },
          .y = [this]{ return m_bar.h() / 2; },

          .textFunc = [this]
          {
              const int    hp = m_creature->getSDHealth().has_value() ? m_creature->getSDHealth()->hp         : 0;
              const int maxHP = m_creature->getSDHealth().has_value() ? m_creature->getSDHealth()->getMaxHP() : 0;

              return str_printf("%*d/%d", std::max<int>(1, to_d(std::to_string(maxHP).size())), hp, maxHP);
          },

          .font
          {
              .id = 15,
              .size = 16,
              .color = colorf::WHITE_A255,
          },

          .attrs
          {
              .show = [this]
              {
                  return showNumber() && m_creature->getSDHealth().has_value();
              },
          },

          .parent{&m_bar},
      }}

    , m_iconArea
      {{
          .x = [this]
          {
              if(showNumber()){
                  return m_bar.dx() + BAR_W_MARGIN;
              }
              return w() / 2 - baseBarWidth() / 2 + HPBuffIcon::buffIconInsetX;
          },

          .w = std::nullopt,
          .h = std::nullopt,

          .parent{this},
      }}
{}

bool HPBarBoard::showNumber() const
{
    return m_runProc->getRuntimeConfig<RTCFG_数字显血>();
}

bool HPBarBoard::showBar() const
{
    if(!m_runProc->getRuntimeConfig<RTCFG_显示血条>()){
        return false;
    }

    if(m_runProc->getRuntimeConfig<RTCFG_满血不显血>()){
        return m_creature->getSDHealth().has_value() && m_creature->getSDHealth()->hp < m_creature->getSDHealth()->getMaxHP();
    }

    return true;
}

int HPBarBoard::barWidthClip() const
{
    return to_d(barWidth() * m_creature->getSDHealth().and_then(&SDHealth::hpRatio).value_or(1.0f));
}

int HPBarBoard::baseBarWidth() const
{
    return SDLDeviceHelper::getTextureWidth(g_progUseDB->retrieve(0X00000014));
}

int HPBarBoard::barWidth() const
{
    return buffIconsPerRow() * HPBuffIcon::buffIconDrawW + 2 * BAR_W_MARGIN;
}

int HPBarBoard::barHeight() const
{
    return SDLDeviceHelper::getTextureHeight(m_frame.getTexture());
}

int HPBarBoard::buffIconsPerRow() const
{
    if(showNumber()){
        return (std::max<int>(BAR_W_MIN, m_numbers.w()) + HPBuffIcon::buffIconDrawW - 1) / HPBuffIcon::buffIconDrawW;
    }
    return HPBuffIcon::buffIconsPerRow;
}

void HPBarBoard::setBuffIDList(const SDBuffIDList &list)
{
    m_iconArea.clearChild();
    m_iconArea.purge();

    for(const auto id: list.idList){
        const auto &br = DBCOM_BUFFRECORD(id);
        fflassert(br);
        if(br.icon.gfxID != SYS_U32NIL && g_progUseDB->retrieve(br.icon.gfxID)){
            new HPBuffIcon
            {{
                .gfxID = br.icon.gfxID,
                .favor = br.favor,

                .rowIcons = [this]
                {
                    return buffIconsPerRow();
                },

                .parent
                {
                    .widget = &m_iconArea,
                    .autoDelete = true,
                },
            }};
        }
    }
}

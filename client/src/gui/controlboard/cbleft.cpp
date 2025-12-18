#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "colorf.hpp"
#include "totype.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "cbleft.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CBLeft::CBLeft(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        ProcessRun *argProc,
        Widget *argParent,
        bool argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = 178,
          .h = 133,

          .attrs
          {
              .inst
              {
                  .show = [](const Widget *self)
                  {
                      return self->hasParent<ControlBoard>()->m_minimize == false;
                  },
              },
          },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)

    , m_bgFull
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000012);
          },
      }}

    , m_bg
      {{
          .getter = &m_bgFull,
          .vr
          {
              0,
              0,
              [this]{ return w(); },
              [this]{ return h(); },
          },
          .parent{this},
      }}

    , m_hpFull
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000018);
          },
      }}

    , m_hp
      {{
          .dir = DIR_DOWNLEFT,
          .x = 33,
          .y = 95,

          .getter = &m_hpFull,
          .vr
          {
              0,
              [this]
              {
                  return m_hpFull.h() - m_hp.gfxCropH();
              },

              m_hpFull.w(),
              [this] -> int
              {
                  if(auto myHero = m_processRun->getMyHero()){
                      return to_dround(m_hpFull.h() * myHero->getHealthRatio().at(0));
                  }
                  return 0;
              },
          },

          .parent{this},
      }}

    , m_mpFull
      {{
          .texLoadFunc = []
          {
              return g_progUseDB->retrieve(0X00000019);
          },
      }}

    , m_mp
      {{
          .dir = DIR_DOWNLEFT,
          .x = 73,
          .y = 95,

          .getter = &m_mpFull,
          .vr
          {
              0,
              [this]
              {
                  return m_mpFull.h() - m_mp.gfxCropH();
              },

              m_mpFull.w(),
              [this]
              {
                  if(auto myHero = m_processRun->getMyHero()){
                      return to_dround(m_mpFull.h() * myHero->getHealthRatio().at(1));
                  }
                  return 0;
              },
          },

          .parent{this},
      }}

    , m_levelBarFull
      {{
          .texLoadFunc = []
          {
              return g_progUseDB->retrieve(0X000000A0);
          },
      }}

    , m_levelBar
      {{
          .dir = DIR_DOWN,
          .x = 153,
          .y = 115,

          .getter = &m_levelBarFull,
          .vr
          {
              0,
              [this]
              {
                  return m_levelBarFull.h() - m_levelBar.gfxCropH();
              },

              m_levelBarFull.w(),
              [this]
              {
                  if(auto myHero = m_processRun->getMyHero()){
                      return to_dround(m_levelBarFull.h() * myHero->getLevelRatio());
                  }
                  return 0;
              },
          },

          .parent{this},
      }}

    , m_inventoryBarFull
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X000000A0);
          },
      }}

    , m_inventoryBar
      {{
          .dir = DIR_DOWN,
          .x = 166,
          .y = 115,

          .getter = &m_inventoryBarFull,
          .vr
          {
              0,
              [this]
              {
                  return m_inventoryBarFull.h() - m_inventoryBar.gfxCropH();
              },

              m_inventoryBarFull.w(),
              [this] -> int
              {
                  if(auto myHero = m_processRun->getMyHero()){
                      return to_dround(m_inventoryBarFull.h() * myHero->getInventoryRatio());
                  }
                  return 0;
              },
          },

          .parent{this},
      }}

    , m_buttonQuickAccess
      {{
          .x = 148,
          .y = 2,

          .texIDList
          {
              .on   = 0X0B000000,
              .down = 0X0B000001,
          },

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("QuickAccessBoard")){
                  p->flipShow();
              }
          },

          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = 8,
          .y = 72,

          .texIDList
          {
              .on   = 0X0000001E,
              .down = 0X0000001F,
          },

          .onTrigger = [](Widget *, int)
          {
              std::exit(0);
          },

          .parent{this},
      }}

    , m_buttonMinize
      {{
          .x = 109,
          .y = 72,

          .texIDList
          {
              .on   = 0X00000020,
              .down = 0X00000021,
          },

          .parent{this},
      }}

    , m_mapGLocFull
      {{
          .textFunc = [this](const Widget *)
          {
              return getMapGLocStr();
          },

          .font
          {
              .id = 1,
              .size = 12,
          },
      }}

    , m_mapGLoc
      {{
          .dir = DIR_NONE,
          .x = 73,
          .y = 117,

          .getter = &m_mapGLocFull,
          .vr
          {
              [this]
              {
                  if(m_mapGLocFull.w() < m_mapGLocMaxWidth){
                      return 0;
                  }
                  return to_d(m_mapGLocPixelSpeed * m_mapGLocAccuTime / 1000.0) % (m_mapGLocFull.w() - m_mapGLocMaxWidth);
              },
              0,

              [this]
              {
                  return std::min<int>(m_mapGLocFull.w(), m_mapGLocMaxWidth);
              },

              [this](const Widget *)
              {
                  return m_mapGLocFull.h();
              },
          },

          .parent{this},
      }}
{}

std::string CBLeft::getMapGLocStr() const
{
    if(uidf::isMap(m_processRun->mapUID())){
        if(const auto &mr = DBCOM_MAPRECORD(m_processRun->mapID())){
            const auto mapNameFull = std::string(to_cstr(mr.name));
            const auto mapNameBase = mapNameFull.substr(0, mapNameFull.find('_'));

            if(auto myHero = m_processRun->getMyHero()){
                return str_printf("%s: %d %d", mapNameBase.c_str(), myHero->x(), myHero->y());
            }
            else{
                return str_printf("%s", mapNameBase.c_str());
            }
        }
    }
    return {};
}

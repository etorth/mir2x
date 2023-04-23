#include <type_traits>
#include "strf.hpp"
#include "uidf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "combatnode.hpp"
#include "processrun.hpp"
#include "pngtexoffdb.hpp"
#include "soundeffectdb.hpp"
#include "inventoryboard.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_equipDB;
extern SoundEffectDB *g_seffDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

TeamStateBoard::TeamStateBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          argX,
          argY,
          0,
          0,

          widgetPtr,
          autoDelete
      }

    , m_enableTeam
      {
          DIR_UPLEFT,
          24,
          47,
          {SYS_U32NIL, 0X00000200, 0X00000201},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }

    , m_createTeam
      {
          DIR_UPLEFT,
          19,
          192,
          {SYS_U32NIL, 0X00000160, 0X00000161},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }

    , m_addMember
      {
          DIR_UPLEFT,
          72,
          192,
          {SYS_U32NIL, 0X00000170, 0X00000171},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }

    , m_deleteMember
      {
          DIR_UPLEFT,
          125,
          192,
          {SYS_U32NIL, 0X00000180, 0X00000181},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }

    , m_refresh
      {
          DIR_UPLEFT,
          177,
          192,
          {SYS_U32NIL, 0X00000190, 0X00000191},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
              m_uidList.clear();
              for(const auto &[uid, coPtr]: m_processRun->getCOList()){
                  if(uid == m_processRun->getMyHeroUID()){
                      continue;
                  }

                  if(uidf::isPlayer(uid)){
                      m_uidList.push_back(uid);
                  }
              }
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }

    , m_close
      {
          DIR_UPLEFT,
          217,
          200,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }

    , m_processRun(runPtr)
{
    setShow(false);
    if(auto texPtr = g_progUseDB->retrieve(0X00000150)){
        std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid team status board frame texture");
    }
}

void TeamStateBoard::drawEx(int, int, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X00000150)){
        g_sdlDevice->drawTexture(texPtr, x(), y());
    }

    XMLTypeset line(m_uidRegionW, LALIGN_LEFT, false, m_font, m_fontSize, m_fontStyle, m_fontColor);
    for(size_t i = 0; const auto uid: m_uidList){
        if(uidf::isPlayer(uid)){
            if(const auto heroPtr = dynamic_cast<Hero *>(m_processRun->findUID(uid, true))){
                const auto name = heroPtr->getName();
                const auto nameText = name.empty() ? std::string("...") : name;

                line.clear();
                line.loadXML(str_printf("<par>%s</par>", nameText.c_str()).c_str());
                line.drawEx(x() + m_startX, y() + m_startY + m_lineSpace / 2 + i * lineHeight(), 0, 0, line.pw(), line.ph());
            }
        }
    }

    m_enableTeam.draw();
    m_createTeam.draw();
    m_addMember.draw();
    m_deleteMember.draw();
    m_refresh.draw();
    m_close.draw();
}

bool TeamStateBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_enableTeam.processEvent(event, valid)){
        return true;
    }

    if(m_createTeam.processEvent(event, valid)){
        return true;
    }

    if(m_addMember.processEvent(event, valid)){
        return true;
    }

    if(m_deleteMember.processEvent(event, valid)){
        return true;
    }

    if(m_refresh.processEvent(event, valid)){
        return true;
    }

    if(m_close.processEvent(event, valid)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y) || focus())){
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, x() + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, y() + event.motion.yrel));
                    moveBy(newX - x(), newY - y());
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

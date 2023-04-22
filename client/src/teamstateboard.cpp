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

PlayerStateBoard::PlayerStateBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
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
          37,
          59,
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
          22,
          193,
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
          77,
          193,
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
          193,
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
          179,
          193,
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

    , m_close
      {
          DIR_UPLEFT,
          220,
          208,
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

void PlayerStateBoard::update(double)
{
}

void PlayerStateBoard::drawEx(int, int, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X00000150)){
        g_sdlDevice->drawTexture(texPtr, x(), y());
    }
}

bool PlayerStateBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_close.processEvent(event, valid)){
        return true;
    }
}

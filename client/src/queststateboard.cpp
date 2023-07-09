#include <type_traits>
#include "strf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "queststateboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

QuestStateBoard::QuestStateBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
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

    , m_lrButton
      {
          DIR_UPLEFT,
          255,
          85,
          {0X00000300, 0X00000300, 0X00000302},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
              //
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

    , m_closeButton
      {
          DIR_UPLEFT,
          255,
          135,
          {0X00000310, 0X00000310, 0X00000312},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
              //
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
    if(auto texPtr = g_progUseDB->retrieve(0X00000350)){
        std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid quest status board frame texture");
    }
}

void QuestStateBoard::update(double)
{
}

void QuestStateBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X00000350)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }
}

bool QuestStateBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_closeButton.processEvent(event, valid)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                return consumeFocus(false);
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "log.hpp"
#include "colorf.hpp"
#include "totype.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"
#include "processrun.hpp"
#include "controlboard.hpp"
#include "clientmonster.hpp"
#include "teamstateboard.hpp"

extern Log *g_log;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CBMiddle::CBMiddle(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarSizeOpt argW,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          std::move(argW),
          0, // reset

          {},

          argParent,
          argAutoDelete,
      }

    , m_processRun(argProc)
    , m_logBoard(hasParent<ControlBoard>()->m_logBoard)

    , m_bg
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::A_SHF(0XFF), drawDstX, drawDstY, self->w(), self->h());
          },

          this,
          false,
      }

    , m_face
      {
          DIR_UPLEFT,
          0,
          0,
          86,
          96,

          argProc,

          this,
          false,
      }

    , m_bgImgFull
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000013);
          },
      }

    , m_bgImg
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return             w(); },
          [this](const Widget *){ return m_bgImgFull.h(); },

          &m_bgImgFull,

          50,
          0,
          287,
          [this](const Widget *){ return m_bgImgFull.h(); },

          this,
          false,
      }

    , m_switchMode
      {
          DIR_UPLEFT,
          [this](const Widget *){ return w() - 17; },
          3,

          {SYS_U32NIL, 0X00000028, 0X00000029},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int clickCount)
          {
              if(auto parptr = hasParent<ControlBoard>()){
                  parptr->onClickSwitchModeButton(clickCount);
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_slider
      {
          DIR_UPLEFT,
          [this](const Widget *){ return w() - 178; },
          40,
          5,
          60,

          false,
          2,
          nullptr,

          this,
          false,
      }


    , m_logView
      {
          DIR_UPLEFT,
          9,
          11,

          std::addressof(m_logBoard),

          0,
          [this](const Widget *) { return std::max<int>(0, to_dround((m_logBoard.h() - 83) * m_slider.getValue())); },
          [this](const Widget *) { return m_logBoard.w(); },
          83,

          {},

          this,
          false,
      }
{
    setH([this]{ return m_bgImgFull.h(); });
}

bool CBMiddle::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(Widget::processEventDefault(event, valid, startDstX, startDstY, roiOpt.value())){
        return true;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            return valid && hasParent<ControlBoard>()->m_cmdLine.consumeFocus(true);
                        }
                    default:
                        {
                            return false;
                        }
                }
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEMOTION:
        default:
            {
                return false;
            }
    }
}

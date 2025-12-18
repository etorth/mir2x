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
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = std::move(argW),

          .attrs
          {
              .inst
              {
                  .show = [](const Widget *self)
                  {
                      if(const auto cb = self->hasParent<ControlBoard>(); !cb->m_minimize && !cb->m_expand){
                          return true;
                      }
                      return false;
                  },

                  .moveOnFocus = false,
                  .afterResize = [](Widget *self)
                  {
                      if(auto middle = dynamic_cast<CBMiddle *>(self)){
                          middle->m_logBoard.setLineWidth(middle->getLogWindowWidth());
                          middle->m_cmdBoard.setLineWidth(0);
                      }
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
    , m_logBoard(hasParent<ControlBoard>()->m_logBoard)
    , m_cmdBoard(hasParent<ControlBoard>()->m_cmdBoard)

    , m_bg
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::A_SHF(0XFF), drawDstX, drawDstY, self->w(), self->h());
          },

          .parent{this},
      }}

    , m_face
      {
          DIR_DOWNRIGHT,
          [this]{ return w() - 19; },
          [this]{ return h() - 20; },

          argProc,

          this,
          false,
      }

    , m_bgImgFull
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000013);
          },
      }}

    , m_bgImg
      {{
          .getter = std::addressof(m_bgImgFull),
          .vr
          {
              50,
              0,
              287,
              m_bgImgFull.h(),
          },

          .resize
          {
              [this]{ return             w() - (m_bgImgFull.w() - 287); },
              [this]{ return m_bgImgFull.h()                          ; },
          },

          .parent{this},
      }}

    , m_switchMode
      {{
          .x = [this]{ return w() - 15; },
          .y = 3,

          .texIDList
          {
              .on   = 0X00000028,
              .down = 0X00000029,
          },

          .onTrigger = [this](Widget *, int clickCount)
          {
              hasParent<ControlBoard>()->onClickSwitchModeButton(clickCount);
          },

          .onClickDone{false},
          .parent{this},
      }}

    , m_slider
      {{
          .bar
          {
              .x = [this]{ return w() - 10; },
              .y = 40,
              .w = 5,
              .h = 60,
          },

          .index = 2,
          .parent{this},
      }}

    , m_logView
      {{
          .x = LOG_WINDOW_X,
          .y = LOG_WINDOW_Y,

          .getter = std::addressof(m_logBoard),
          .vr
          {
              0,
              [this]{ return std::max<int>(0, to_dround((m_logBoard.h() - LOG_WINDOW_HEIGHT) * m_slider.getValue())); },
              [this]{ return getLogWindowWidth(); },
              LOG_WINDOW_HEIGHT,
          },

          .parent{this},
      }}

    , m_cmdView
      {{
          .x = CMD_WINDOW_X,
          .y = CMD_WINDOW_Y,

          .getter = std::addressof(m_cmdBoard),
          .vr
          {
              [this]{ return m_cmdBoardCropX ; },
              0,

              [this]{ return getCmdWindowWidth(); },
              CMD_WINDOW_HEIGHT,
          },

          .parent{this},
      }}
{
    setH([this]{ return m_bgImgFull.h(); });

    moveFront(&m_cmdView);
    moveFront(&m_logView);
    moveFront(&m_face);
    moveFront(&m_bg);
}

bool CBMiddle::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(Widget::processEventDefault(event, valid, m)){
        return true;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(!valid){
                    return false;
                }

                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            return hasParent<ControlBoard>()->m_cmdBoard.consumeFocus(true);
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

void CBMiddle::onCmdCR()
{
    m_cmdBoardCropX = 0;
}

void CBMiddle::onCmdCursorMove()
{
    const auto cmdBoardCropW = getCmdWindowWidth();
    const auto cursorROI = Widget::makeROI(m_cmdBoard.getCursorPLoc());

    if(cursorROI.x < m_cmdBoardCropX){
        m_cmdBoardCropX = cursorROI.x;
    }

    if(cursorROI.x + cursorROI.w > m_cmdBoardCropX + cmdBoardCropW){
        m_cmdBoardCropX = cursorROI.x + cursorROI.w - cmdBoardCropW;
    }
}

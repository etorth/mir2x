#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "controlboard.hpp"
#include "cbmiddleexpand.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CBMiddleExpand::CBMiddleExpand(
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
                      if(const auto cb = self->hasParent<ControlBoard>(); !cb->m_minimize && cb->m_expand){
                          return true;
                      }
                      return false;
                  },

                  .afterResize = [](Widget *self)
                  {
                      if(auto expanded = dynamic_cast<CBMiddleExpand *>(self)){
                          expanded->m_logBoard.setLineWidth(expanded->getLogWindowWidth());
                          expanded->m_cmdBoard.setLineWidth(expanded->getCmdWindowWidth());
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
              g_sdlDevice->fillRectangle(colorf::A_SHF(0XF0), drawDstX, drawDstY, self->w(), self->h());
          },

          .parent{this},
      }}

    , m_bgImgFull
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(0X00000027); },
      }}

    , m_bgImg
      {{
          .getter = std::addressof(m_bgImgFull),
          .vr
          {
              50,
              47,
              287,
              196,
          },

          .resize
          {
              [this]{ return w() - (m_bgImgFull.w() - 287); },
              [this]{ return h() - (m_bgImgFull.h() - 196); },
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

    , m_buttonEmoji
      {{
          .x = [this]{ return w() - 90; },
          .y = 87,

          .texIDList
          {
              .on   = 0X00000023,
              .down = 0X00000024,
          },

          .parent{this},
      }}

    , m_buttonMute
      {{
          .x = [this]{ return w() - 55; },
          .y = 87,

          .texIDList
          {
              .on   = 0X00000025,
              .down = 0X00000026,
          },

          .parent{this},
      }}

    , m_slider
      {{
           .bar
           {
              .x = [this]{ return w() - 10; },
              .y = 40,
              .w = 5,
              .h = [this]{ return h() - m_bgImgFull.h() + 223; }
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
              [this]{ return std::max<int>(0, to_dround((m_logBoard.h() - getLogWindowHeight()) * m_slider.getValue())); },
              [this]{ return getLogWindowWidth (); },
              [this]{ return getLogWindowHeight(); },
          },

          .parent{this},
      }}

    , m_cmdView
      {{
          .x = CMD_WINDOW_X,
          .y = [this]{ return CMD_WINDOW_Y + (h() - m_bgImgFull.h()); },

          .getter = std::addressof(m_cmdBoard),
          .vr
          {
              0,
              [this]{ return m_cmdBoardCropY ; },

              [this]{ return getCmdWindowWidth(); },
              CMD_WINDOW_HEIGHT,
          },

          .parent{this},
      }}
{
    setH([this]
    {
        if(const auto cb = hasParent<ControlBoard>(); !cb->m_minimize && cb->m_expand){
            if(cb->m_maximize){
                return g_sdlDevice->getRendererHeight();
            }
            else{
                return std::min<int>(400, g_sdlDevice->getRendererHeight());
            }
        }
        return 0;
    });

    moveFront(&m_cmdView);
    moveFront(&m_logView);
    moveFront(&m_bg);
}

bool CBMiddleExpand::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
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
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            return valid && hasParent<ControlBoard>()->m_cmdBoard.consumeFocus(true);
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

void CBMiddleExpand::onCmdCR()
{
    m_cmdBoardCropY = 0;
}

void CBMiddleExpand::onCmdCursorMove()
{
    const auto cmdBoardCropH = CMD_WINDOW_HEIGHT;
    const auto cursorROI = Widget::makeROI(m_cmdBoard.getCursorPLoc());

    if(cursorROI.y < m_cmdBoardCropY){
        m_cmdBoardCropY = cursorROI.y;
    }

    if(cursorROI.y + cursorROI.h > m_cmdBoardCropY + cmdBoardCropH){
        m_cmdBoardCropY = cursorROI.y + cursorROI.h - cmdBoardCropH;
    }
}

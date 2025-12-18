#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "horseboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

HorseBoard::HorseBoard(
        dir8_t argDir,

        int argX,
        int argY,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,

          .w = 257,
          .h = 322,

          .parent
          {
              argParent,
              argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_greyBg
      {{
          .w = [this](const Widget *){ return w(); },
          .h = [this](const Widget *){ return h(); },

          .drawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::GREY + colorf::A_SHF(255), drawDstX, drawDstY, self->w(), self->h());
          },

          .parent{this},
      }}

    , m_imageBg
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000700);
          },

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_close
      {{
          .x = 217,
          .y = 278,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
          },

          .parent{this},
      }}

    , m_up
      {{
          .x = 20,
          .y = 231,

          .texIDList
          {
              .on   = 0X00000710,
              .down = 0X00000711,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_down
      {{
          .x = 77,
          .y = 231,

          .texIDList
          {
              .on   = 0X00000720,
              .down = 0X00000721,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_hide
      {{
          .x = 134,
          .y = 231,

          .texIDList
          {
              .on   = 0X00000730,
              .down = 0X00000731,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_show
      {{
          .x = 191,
          .y = 231,

          .texIDList
          {
              .on   = 0X00000740,
              .down = 0X00000741,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}
{
    setShow(false);
}

bool HorseBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_close.processEventParent(event, valid, m)){ return true; }
    if(m_up   .processEventParent(event, valid, m)){ return true; }
    if(m_down .processEventParent(event, valid, m)){ return true; }
    if(m_hide .processEventParent(event, valid, m)){ return true; }
    if(m_show .processEventParent(event, valid, m)){ return true; }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        {
                            setShow(false);
                            setFocus(false);
                            return true;
                        }
                    default:
                        {
                            return consumeFocus(false);
                        }
                }
            }
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || focus())){
                    const auto remapXDiff = m.x - m.ro->x;
                    const auto remapYDiff = m.y - m.ro->y;

                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + event.motion.yrel));

                    moveBy(newX - remapXDiff, newY - remapYDiff);
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                return consumeFocus(true);
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

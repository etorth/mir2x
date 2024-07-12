#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "horseboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

HorseBoard::HorseBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          argX,
          argY,
          257,
          322,
          {},

          widgetPtr,
          autoDelete,
      }

    , m_processRun(runPtr)
    , m_close
      {
          DIR_UPLEFT,
          217,
          278,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
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
      }

    , m_up
      {
          DIR_UPLEFT,
          20,
          231,
          {SYS_U32NIL, 0X00000710, 0X00000711},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }

    , m_down
      {
          DIR_UPLEFT,
          77,
          231,
          {SYS_U32NIL, 0X00000720, 0X00000721},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }

    , m_hide
      {
          DIR_UPLEFT,
          134,
          231,
          {SYS_U32NIL, 0X00000730, 0X00000731},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }

    , m_show
      {
          DIR_UPLEFT,
          191,
          231,
          {SYS_U32NIL, 0X00000740, 0X00000741},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }
{
    setShow(false);
}


void HorseBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    g_sdlDevice->fillRectangle(colorf::GREY + colorf::A_SHF(255), dstX, dstY, 257, 322);
    if(auto texPtr = g_progUseDB->retrieve(0X00000700)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }

    m_close.draw();
    m_up   .draw();
    m_down .draw();
    m_hide .draw();
    m_show .draw();
}

bool HorseBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_close.processEvent(event, valid)){ return true; }
    if(m_up   .processEvent(event, valid)){ return true; }
    if(m_down .processEvent(event, valid)){ return true; }
    if(m_hide .processEvent(event, valid)){ return true; }
    if(m_show .processEvent(event, valid)){ return true; }

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

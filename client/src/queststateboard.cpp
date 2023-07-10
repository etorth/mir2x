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

    , m_slider
      {
          DIR_UPLEFT,
          253,
          160,
          9,
          214,

          false,
          3,
          nullptr,

          this,
          false,
      }

    , m_lrButton
      {
          DIR_UPLEFT,
          242,
          76,
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
          243,
          108,
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

    m_slider.draw();
    m_lrButton.draw();
    m_closeButton.draw();
}

bool QuestStateBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_slider.processEvent(event, valid)){
        return true;
    }

    if(m_lrButton.processEvent(event, valid)){
        return true;
    }

    if(m_closeButton.processEvent(event, valid)){
        return true;
    }

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

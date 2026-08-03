#include <algorithm>
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "acutionregisterboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

AcutionRegisterBoard::AcutionRegisterBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .dir = DIR_NONE,
          .x = [](const Widget *){ return g_sdlDevice->getRendererWidth () / 2; },
          .y = [](const Widget *){ return g_sdlDevice->getRendererHeight() / 2; },
          .w = 408,
          .h = 286,
          .parent{argParent, argAutoDelete},
      }}

    , m_runProc(argProc)
    , m_background
      {{
          .texLoadFunc = [](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00001410);
          },
          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_buttonRegister
      {{
          .x = 248,
          .y = 229,
          .texIDList
          {
              .on   = 0X000000B3,
              .down = 0X000000B4,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonCancel
      {{
          .x = 318,
          .y = 229,
          .texIDList
          {
              .on   = 0X00000850,
              .down = 0X00000851,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = 368,
          .y = 241,
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
{
    fflassert(m_runProc);
    setShow(false);
}

bool AcutionRegisterBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_buttonRegister.processEventParent(event, valid, m)){ return true; }
    if(m_buttonCancel  .processEventParent(event, valid, m)){ return true; }
    if(m_buttonClose   .processEventParent(event, valid, m)){ return true; }

    switch(event.type){
        case SDL_EVENT_KEY_DOWN:
            {
                if(event.key.key == SDLK_ESCAPE){
                    setShow(false);
                    setFocus(false);
                    return true;
                }
                return consumeFocus(false);
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                return consumeFocus(m.in(to_d(event.button.x), to_d(event.button.y)));
            }
        case SDL_EVENT_MOUSE_MOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(to_d(event.motion.x), to_d(event.motion.y)) || focus())){
                    const int remapX = m.x - m.ro->x;
                    const int remapY = m.y - m.ro->y;
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();

                    const int newX = std::clamp(remapX + to_d(event.motion.xrel), 0, rendererW - w());
                    const int newY = std::clamp(remapY + to_d(event.motion.yrel), 0, rendererH - h());
                    moveBy(newX - remapX, newY - remapY);
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

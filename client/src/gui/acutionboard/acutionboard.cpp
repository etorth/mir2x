#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "acutionboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

AcutionBoard::AcutionBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .dir = DIR_NONE,

          .x = [](const Widget *){ return g_sdlDevice->getRendererWidth () / 2; },
          .y = [](const Widget *){ return g_sdlDevice->getRendererHeight() / 2; },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_runProc(argProc)
    , m_background
      {{
          .texLoadFunc = [](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00001400);
          },

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}
{}

#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "acutionboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

AcutionBoard::AcutionBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {
          DIR_NONE,
          [](const Widget *){ return g_sdlDevice->getRendererWidth () / 2; },
          [](const Widget *){ return g_sdlDevice->getRendererHeight() / 2; },

          0,
          0,

          {},

          argParent,
          argAutoDelete
      }

    , m_runProc(argProc)
    , m_background
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const ImageBoard *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00001400);
          },

          false,
          false,
          false,

          colorf::WHITE + colorf::A_SHF(0XFF),

          this,
          false,
      }
{}

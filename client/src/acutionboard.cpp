#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "acutionboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

AcutionBoard::AcutionBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        ProcessRun *argProc,

        Widget * argParent,
        bool     argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

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

#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "guildboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

GuildBoard::GuildBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          argX,
          argY,
          594,
          444,
          {},

          widgetPtr,
          autoDelete,
      }

    , m_processRun(runPtr)
    , m_closeButton
      {
          DIR_UPLEFT,
          554,
          399,
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
          false,
          true,

          this,
      }

    , m_announcement
      {
          DIR_UPLEFT,
          40,
          385,
          {SYS_U32NIL, 0X00000510, 0X00000511},
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
          false,
          true,

          this,
      }

    , m_members
      {
          DIR_UPLEFT,
          90,
          385,
          {SYS_U32NIL, 0X00000520, 0X00000521},
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
          false,
          true,

          this,
      }

    , m_chat
      {
          DIR_UPLEFT,
          140,
          385,
          {SYS_U32NIL, 0X00000530, 0X00000531},
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
          false,
          true,

          this,
      }

    , m_editAnnouncement
      {
          DIR_UPLEFT,
          290,
          385,
          {SYS_U32NIL, 0X00000540, 0X00000541},
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
          false,
          true,

          this,
      }

    , m_removeMember
      {
          DIR_UPLEFT,
          340,
          385,
          {SYS_U32NIL, 0X00000550, 0X00000551},
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
          false,
          true,

          this,
      }

    , m_disbandGuild
      {
          DIR_UPLEFT,
          390,
          385,
          {SYS_U32NIL, 0X00000560, 0X00000561},
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
          false,
          true,

          this,
      }

    , m_editMemberPosition
      {
          DIR_UPLEFT,
          440,
          385,
          {SYS_U32NIL, 0X00000570, 0X00000571},
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
          false,
          true,

          this,
      }

    , m_dissolveCovenant
      {
          DIR_UPLEFT,
          490,
          385,
          {SYS_U32NIL, 0X00000580, 0X00000581},
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
          false,
          true,

          this,
      }

    , m_slider
      {
          DIR_UPLEFT,
          564,
          62,
          9,
          277,

          false,
          3,
          nullptr,

          this,
          false,
      }
{
    setShow(false);
}


void GuildBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X00000500)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }

    m_closeButton       .draw();
    m_announcement      .draw();
    m_members           .draw();
    m_chat              .draw();
    m_editAnnouncement  .draw();
    m_removeMember      .draw();
    m_disbandGuild      .draw();
    m_editMemberPosition.draw();
    m_dissolveCovenant  .draw();
    m_slider            .draw();
}

bool GuildBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_closeButton       .processEvent(event, valid)){ return true; }
    if(m_announcement      .processEvent(event, valid)){ return true; }
    if(m_members           .processEvent(event, valid)){ return true; }
    if(m_chat              .processEvent(event, valid)){ return true; }
    if(m_editAnnouncement  .processEvent(event, valid)){ return true; }
    if(m_removeMember      .processEvent(event, valid)){ return true; }
    if(m_disbandGuild      .processEvent(event, valid)){ return true; }
    if(m_editMemberPosition.processEvent(event, valid)){ return true; }
    if(m_dissolveCovenant  .processEvent(event, valid)){ return true; }
    if(m_slider            .processEvent(event, valid)){ return true; }

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

#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "guildboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

GuildBoard::GuildBoard(int argX, int argY, ProcessRun *runPtr, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .x = argX,
          .y = argY,
          .w = 594,
          .h = 444,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(runPtr)
    , m_bg
      {{
          .texLoadFunc = [](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00000500);
          },

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_closeButton
      {{
          .x = 554,
          .y = 399,

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

    , m_announcement
      {{
          .x = 40,
          .y = 385,

          .texIDList
          {
              .on   = 0X00000510,
              .down = 0X00000511,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_members
      {{
          .x = 90,
          .y = 385,

          .texIDList
          {
              .on   = 0X00000520,
              .down = 0X00000521,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_chat
      {{
          .x = 140,
          .y = 385,

          .texIDList
          {
              .on   = 0X00000530,
              .down = 0X00000531,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_editAnnouncement
      {{
          .x = 290,
          .y = 385,

          .texIDList
          {
              .on   = 0X00000540,
              .down = 0X00000541,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_removeMember
      {{
          .x = 340,
          .y = 385,

          .texIDList
          {
              .on   = 0X00000550,
              .down = 0X00000551,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_disbandGuild
      {{
          .x = 390,
          .y = 385,

          .texIDList
          {
              .on   = 0X00000560,
              .down = 0X00000561,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_editMemberPosition
      {{
          .x = 440,
          .y = 385,

          .texIDList
          {
              .on   = 0X00000570,
              .down = 0X00000571,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_dissolveCovenant
      {{
          .x = 490,
          .y = 385,

          .texIDList
          {
              .on   = 0X00000580,
              .down = 0X00000581,
          },

          .onTrigger = [this](Widget *, int)
          {
          },

          .parent{this},
      }}

    , m_slider
      {{
          .bar
          {
              .x = 564,
              .y = 53,
              .w = 9,
              .h = 294,
          },

          .index = 3,
          .parent{this},
      }}
{
    setShow(false);
}

bool GuildBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_closeButton       .processEventParent(event, valid, m)){ return true; }
    if(m_announcement      .processEventParent(event, valid, m)){ return true; }
    if(m_members           .processEventParent(event, valid, m)){ return true; }
    if(m_chat              .processEventParent(event, valid, m)){ return true; }
    if(m_editAnnouncement  .processEventParent(event, valid, m)){ return true; }
    if(m_removeMember      .processEventParent(event, valid, m)){ return true; }
    if(m_disbandGuild      .processEventParent(event, valid, m)){ return true; }
    if(m_editMemberPosition.processEventParent(event, valid, m)){ return true; }
    if(m_dissolveCovenant  .processEventParent(event, valid, m)){ return true; }
    if(m_slider            .processEventParent(event, valid, m)){ return true; }

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

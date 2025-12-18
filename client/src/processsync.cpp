#include "log.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "fontstyle.hpp"
#include "processsync.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ProcessSync::ProcessSync()
    : Process()
    , m_canvas
      {{
          .w = 800,
          .h = 600,
      }}

    , m_barFull
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(0X00000002); },
      }}

    , m_bar
      {{
          .x = 112,
          .y = 528,

          .getter = &m_barFull,
          .vr
          {
              [this]{ return m_barFull.w() * m_ratio / 100; },
              [this]{ return m_barFull.h()                ; },
          },

          .parent{&m_canvas},
      }}

    , m_bgImg
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000001); },
          .parent{&m_canvas},
      }}

    , m_barText
      {{
          .dir = DIR_NONE,

          .x = m_bar.dx() + (m_barFull.w() / 2),
          .y = m_bar.dy() + (m_bar    .h() / 2),

          .textFunc = "Connecting...",
          .font
          {
              .id = 1,
              .size = 10,
          },

          .parent{&m_canvas},
      }}
{}

void ProcessSync::processEvent(const SDL_Event &event)
{
    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    g_client->requestProcess(PROCESSID_LOGIN);
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

void ProcessSync::update(double fUpdateTime)
{
    if(m_ratio >= 100){
        g_client->requestProcess(PROCESSID_LOGIN);
        return;
    }

    m_ratio += (fUpdateTime > 0.0 ? 1 : 0);
}

void ProcessSync::draw() const
{
    const SDLDeviceHelper::RenderNewFrame newFrame;
    m_canvas.drawRoot({});
}

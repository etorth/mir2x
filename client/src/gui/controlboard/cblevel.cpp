#include "bevent.hpp"
#include "cblevel.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"

extern SDLDevice *g_sdlDevice;

CBLevel::CBLevel(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        ProcessRun *argProc,
        Button::TriggerCBFunc argOnClick,

        Widget *argParent,
        bool    argAutoDelete)

    : TrigfxButton
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .onTrigger = std::move(argOnClick),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_canvas
      {{
          .w = 16,
          .h = 16,

          .childList
          {
              {
                  .widget = new ImageBoard
                  {{
                      .texLoadFunc = []
                      {
                          return g_sdlDevice->getCover(8, 360);
                      },

                      .modColor = [this] -> uint32_t
                      {
                          switch(getState()){
                              case BEVENT_ON  : return colorf::BLUE + colorf::A_SHF(0XFF);
                              case BEVENT_DOWN: return colorf::RED  + colorf::A_SHF(0XFF);
                              default         : return 0;
                          }
                      },
                  }},

                  .dir = DIR_NONE,

                  .x = [this]{ return m_canvas.w() / 2; },
                  .y = [this]{ return m_canvas.h() / 2; },

                  .autoDelete = true,
              },

              {
                  .widget = new TextBoard
                  {{
                      .textFunc = [this] -> std::string
                      {
                          return std::to_string(m_processRun->getMyHero()->getLevel());
                      },

                      .font
                      {
                          .id = 0,
                          .size = 12,
                          .color = colorf::YELLOW + colorf::A_SHF(255),
                      },
                  }},

                  .dir = DIR_NONE,

                  .x = [this]{ return m_canvas.w() / 2; },
                  .y = [this]{ return m_canvas.h() / 2; },

                  .autoDelete = true,
              },
          },
      }}
{
    setGfxFunc([this]{ return &m_canvas; });
}

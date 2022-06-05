#include "widget.hpp"
#include "levelbox.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "imageboard.hpp"

extern SDLDevice *g_sdlDevice;
LevelBox::LevelBox(
        dir8_t dir,
        int x,
        int y,

        ProcessRun *proc,
        const std::function<void(int)> &onDrag,
        const std::function<void(   )> &onDoubleClick,

        Widget *parent,
        bool    autoDelete)
    : Widget(dir, x, y, 16, 16, parent, autoDelete)
    , m_processRun(proc)
    , m_onDrag(onDrag)
    , m_onDoubleClick(onDoubleClick)
{}

bool LevelBox::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!in(event.button.x, event.button.y)){
                    m_state = BEVENT_OFF;
                    return false;
                }

                if(event.button.clicks == 2){
                    m_onDoubleClick();
                }
                m_state = BEVENT_DOWN;
                return true;
            }
        case SDL_MOUSEBUTTONUP:
            {
                if(!in(event.button.x, event.button.y)){
                    m_state = BEVENT_OFF;
                    return false;
                }

                m_state = BEVENT_ON;
                return true;
            }
        case SDL_MOUSEMOTION:
            {
                // even not in the box
                // we still need to drag the widget

                if(m_state == BEVENT_DOWN){
                    if(event.motion.state & SDL_BUTTON_LMASK){
                        m_onDrag(event.motion.yrel);
                        return true;
                    }
                    else{
                        if(in(event.motion.x, event.motion.y)){
                            m_state = BEVENT_ON;
                            return true;
                        }
                        else{
                            m_state = BEVENT_OFF;
                            return false;
                        }
                    }
                }

                if(in(event.motion.x, event.motion.y)){
                    m_state = BEVENT_ON;
                    return true;
                }
                else{
                    m_state = BEVENT_OFF;
                    return false;
                }
            }
        default:
            {
                return false;
            }
    }
}

void LevelBox::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    class DrawHelper: public WidgetGroup
    {
        private:
            LabelBoard m_label;
            ImageBoard m_image;

        public:
            DrawHelper(int argX, int argY, int argW, int argH, uint32_t color, uint32_t level)
                : WidgetGroup
                  {
                      DIR_UPLEFT,
                      argX,
                      argY,
                      argW,
                      argH,
                  }

                , m_label
                  {
                      DIR_NONE,
                      w() / 2,
                      h() / 2,

                      to_u8cstr(str_printf(u8"%d", to_d(level))),

                      0,
                      12,
                      0,
                      colorf::YELLOW + colorf::A_SHF(255),

                      this,
                      false,
                  }

                , m_image
                  {
                      DIR_NONE,
                      w() / 2,
                      h() / 2,

                      [](const ImageBoard *)
                      {
                          return g_sdlDevice->getCover(8, 360);
                      },

                      color,
                      this,
                      false,
                  }
            {}
    };

    DrawHelper
    {
        x(),
        y(),
        w(),
        h(),

        [this]() -> uint32_t
        {
            switch(m_state){
                case BEVENT_ON  : return colorf::BLUE + colorf::A_SHF(0XFF);
                case BEVENT_DOWN: return colorf::RED  + colorf::A_SHF(0XFF);
                default         : return 0;
            }
        }(),
        m_processRun->getMyHero()->getLevel(),
    }.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}

#include "pngtexdb.hpp"
#include "checkbox.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CheckBox::CheckBox(dir8_t argDir,
        int argX,
        int argY,
        int argW,
        int argH,

        uint32_t argColor,

        std::function<bool(const Widget *      )> argValGetter,
        std::function<void(      Widget *, bool)> argValSetter,
        std::function<void(      Widget *, bool)> argValOnChange,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      ({
          .dir = argDir,

          .x = argX,
          .y = argY,
          .w = argW,
          .h = argH,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      })

    , m_color(argColor)

    , m_valGetter  (std::move(argValGetter  ))
    , m_valSetter  (std::move(argValSetter  ))
    , m_valOnChange(std::move(argValOnChange))

    , m_img
      {{
          .dir = DIR_NONE,
          .x = [this](const Widget *){ return w() / 2; },
          .y = [this](const Widget *){ return h() / 2; },

          .texLoadFunc = [](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00000480);
          },

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_box
      {{
          .w = [this](const Widget *){ return w(); },
          .h = [this](const Widget *){ return h(); },

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
          {
              // +----1----+
              // |        ||
              // 4        52
              // |        ||
              // |----6---+|
              // +----3----+

              const auto  solidColor = m_color;
              const auto shadowColor = colorf::maskRGB(m_color) + colorf::A_SHF(colorf::A(m_color) / 2);

              g_sdlDevice->drawLine( solidColor, drawDstX +       0, drawDstY +       0, drawDstX + w() - 1, drawDstY +       0); // 1
              g_sdlDevice->drawLine( solidColor, drawDstX + w() - 1, drawDstY +       0, drawDstX + w() - 1, drawDstY + h() - 1); // 2
              g_sdlDevice->drawLine( solidColor, drawDstX +       0, drawDstY + h() - 1, drawDstX + w() - 1, drawDstY + h() - 1); // 3
              g_sdlDevice->drawLine( solidColor, drawDstX +       0, drawDstY +       0, drawDstX +       0, drawDstY + h() - 1); // 4
              g_sdlDevice->drawLine(shadowColor, drawDstX + w() - 2, drawDstY +       0, drawDstX + w() - 2, drawDstY + h() - 2); // 5
              g_sdlDevice->drawLine(shadowColor, drawDstX +       0, drawDstY + h() - 2, drawDstX + w() - 2, drawDstY + h() - 2); // 6
          },

          .parent{this},
      }}
{
    m_img.setShow([this](const Widget *){ return getter(); });
}

bool CheckBox::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(!m.calibrate(this)){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                return consumeFocus(m.in(event.button.x, event.button.y));
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m.in(event.button.x, event.button.y)){
                    toggle();
                    return consumeFocus(true);
                }
                else{
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                return consumeFocus(m.in(event.motion.x, event.motion.y));
            }
        case SDL_KEYUP:
            {
                return consumeFocus(focus());
            }
        case SDL_KEYDOWN:
            {
                if(focus()){
                    switch(event.key.keysym.sym){
                        case SDLK_SPACE:
                        case SDLK_RETURN:
                            {
                                toggle();
                                return consumeFocus(true);
                            }
                        default:
                            {
                                return true;
                            }
                    }
                }
                else{
                    return false;
                }
            }
        default:
            {
                return false;
            }
    }
}

void CheckBox::toggle()
{
    setter(!getter());
    if(m_valOnChange){
        m_valOnChange(this, getter());
    }
}

bool CheckBox::getter() const
{
    if(m_valGetter){
        return m_valGetter(this);
    }
    else{
        return m_innVal;
    }
}

void CheckBox::setter(bool value)
{
    if(!m_valGetter){
        m_innVal = value;
    }

    if(m_valSetter){
        m_valSetter(this, value);
    }
}

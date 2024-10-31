#include <initializer_list>
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "menubutton.hpp"

extern SDLDevice *g_sdlDevice;

MenuButton::MenuButton(dir8_t argDir,
        int argX,
        int argY,

        std::pair<Widget *, bool> argGfxWidget,
        std::pair<Widget *, bool> argMenuBoard,

        std::array<int, 4> argMargin,

        Widget *argParent,
        bool argAutoDelete)
    : Widget
      {
          argDir,
          argX,
          argY,
          0,
          0,
          {},

          argParent,
          argAutoDelete,
      }

    , m_margin(argMargin)

    , m_gfxWidget(argGfxWidget.first)
    , m_menuBoard(argMenuBoard.first)

    , m_button
      {
          DIR_UPLEFT,
          m_margin[2],
          m_margin[0],
          m_gfxWidget->w() + m_margin[2] + m_margin[3],
          m_gfxWidget->h() + m_margin[0] + m_margin[1],

          nullptr,
          nullptr,
          [this](Widget *)
          {
              m_menuBoard->flipShow();
              updateMenuButtonSize();
          },

          SYS_U32NIL,
          SYS_U32NIL,
          SYS_U32NIL,

          0,
          0,
          1,
          1,

          false,
          false,

          this,
          false,
      }
{
    m_gfxWidget->moveAt(DIR_UPLEFT, m_margin[2], m_margin[0]);
    m_menuBoard->moveAt(DIR_UPLEFT, m_margin[2], m_margin[0] + m_gfxWidget->h());

    addChild(argGfxWidget.first, argGfxWidget.second);
    addChild(argMenuBoard.first, argMenuBoard.second);

    m_menuBoard->setShow(false);
    updateMenuButtonSize();
}

void MenuButton::updateMenuButtonSize()
{
    setW(m_margin[2]                    + std::max<int>(m_gfxWidget->w() + m_margin[3], m_menuBoard->show() ? m_menuBoard->w() : 0));
    setH(m_margin[0] + m_gfxWidget->h() + std::max<int>(                   m_margin[1], m_menuBoard->show() ? m_menuBoard->h() : 0));
}

void MenuButton::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    for(auto widget: {m_gfxWidget, m_menuBoard}){
        if(!widget->show()){
            continue;
        }

        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        if(!mathf::cropChildROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    widget->dx(),
                    widget->dy(),
                    widget-> w(),
                    widget-> h())){
            continue;
        }

        widget->drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
        if(widget == m_gfxWidget){
            switch(m_button.getState()){
                case BEVENT_ON  : g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(128), dstXCrop, dstYCrop, srcWCrop, srcHCrop); break;
                case BEVENT_DOWN: g_sdlDevice->fillRectangle(colorf::RED   + colorf::A_SHF(128), dstXCrop, dstYCrop, srcWCrop, srcHCrop); break;
                default: break;
            }
        }
    }
}

bool MenuButton::processEventDefault(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(Widget::processEvent(event, valid)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m_menuBoard->in(event.button.x, event.button.y)){
                    m_menuBoard->setShow(false);
                }
                return consumeFocus(false);
            }
        default:
            {
                return false;
            }
    }
}

#include "sdldevice.hpp"
#include "shapeclipboard.hpp"

ShapeClipBoard::ShapeClipBoard(Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,
        Widget::VarSize argH,

        std::function<void(const Widget *, int, int)> argDrawFunc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          std::move(argW),
          std::move(argH),

          {},

          argParent,
          argAutoDelete,
      }

    , m_drawFunc(std::move(argDrawFunc))
{}

void ShapeClipBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    if(!m_drawFunc){
        return;
    }

    const SDLDeviceHelper::EnableRenderClipRectangle enableClip(dstX, dstY, srcW, srcH);
    m_drawFunc(this, dstX - srcX, dstY - srcY);
}

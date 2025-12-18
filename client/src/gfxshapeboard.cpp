#include "sdldevice.hpp"
#include "gfxshapeboard.hpp"

GfxShapeBoard::GfxShapeBoard(GfxShapeBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = std::move(args.w),
          .h = std::move(args.h),

          .attrs
          {
              .inst = std::move(args.attrs),
          },
          .parent = std::move(args.parent),
      }}

    , m_drawFunc(std::move(args.drawFunc))
{}

void GfxShapeBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(Widget::hasDrawFunc(m_drawFunc)){
        const SDLDeviceHelper::EnableRenderCropRectangle enableClip(m.x, m.y, m.ro->w, m.ro->h);
        Widget::execDrawFunc(m_drawFunc, this, m.x - m.ro->x, m.y - m.ro->y);
    }
}

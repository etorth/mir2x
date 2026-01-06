#include "gfxdirbutton.hpp"

GfxDirButton::GfxDirButton(GfxDirButton::InitArgs args)
    : TrigfxButton
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .onTrigger = std::move(args.onTrigger),
          .parent = std::move(args.parent),
      }}

    , m_gfxDrawer
      {{
          .w = std::move(args.w),
          .h = std::move(args.h),

          .drawFunc = [triangle = std::move(args.triangle), frame = std::move(args.frame), this](const Widget *self, int drawDstX, int drawDstY)
          {
              const auto tw = Widget::evalSize(triangle.w, self);
              const auto th = Widget::evalSize(triangle.h, self);

              int x1 = 0, y1 = 0; //        apex vertex of the isosceles triangle
              int x2 = 0, y2 = 0; // first  base vertex
              int x3 = 0, y3 = 0; // second base vertex

              switch(const auto d = Widget::evalDir(triangle.dir, self)){
                  case DIR_UP   : x1 = tw / 2; y1 =  0    ; x2 =  0; y2 = th; x3 = tw; y3 = th; break;
                  case DIR_DOWN : x1 = tw / 2; y1 = th    ; x2 =  0; y2 =  0; x3 = tw; y3 =  0; break;
                  case DIR_LEFT : x1 = 0     ; y1 = th / 2; x2 = tw; y2 =  0; x3 = tw; y3 = th; break;
                  case DIR_RIGHT: x1 = tw    ; y1 = th / 2; x2 =  0; y2 =  0; x3 =  0; y3 = th; break;
                  default: throw fflerror("invalid direction: %s", pathf::dirName(d));
              }

              g_sdlDeive->fillTriangle(Widget::evalU32(color, self),
                      dstDrawX + x1, dstDrawY + y1,
                      dstDrawX + x2, dstDrawY + y2,
                      dstDrawX + x3, dstDrawY + y3);

              if(Widget::evalBool(frame.show, self)){
                  g_sdlDevice->drawRectangle(Widget::evalU32(frame.color, this), dstDrawX, dstDrawY, Widget::evalSize(frame.w, self), Widget::evalSize(frame.h, self));
              }
          },
      }}
{
    setGfxFunc([this]{ return &m_gfxDrawer; });
}

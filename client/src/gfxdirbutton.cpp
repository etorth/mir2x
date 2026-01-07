#include "pathf.hpp"
#include "bevent.hpp"
#include "sdldevice.hpp"
#include "gfxdirbutton.hpp"

extern SDLDevice *g_sdlDevice;

GfxDirButton::GfxDirButton(GfxDirButton::InitArgs args)
    : TrigfxButton
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .onTrigger = std::move(args.onTrigger),
          .parent    = std::move(args.parent),
      }}

    , m_gfxDrawer
      {{
          .w = std::move(args.w),
          .h = std::move(args.h),

          .drawFunc = [triangle = std::move(args.triangle), frame = std::move(args.frame), this](const Widget *self, int dstDrawX, int dstDrawY)
          {
              const auto state = this->getState();

              const auto cw = self->w(); // canvas w
              const auto ch = self->h(); // canvas h

              const auto tw = Widget::evalSizeOpt(triangle.w, self, [cw]{ return cw; });
              const auto th = Widget::evalSizeOpt(triangle.h, self, [ch]{ return ch; });

              int x1 = 0, y1 = 0; //        apex vertex of the isosceles triangle
              int x2 = 0, y2 = 0; // first  base vertex
              int x3 = 0, y3 = 0; // second base vertex

              switch(const auto d = Widget::evalDir(triangle.dir, self)){
                  case DIR_UP   : x1 = tw / 2; y1 =  0    ; x2 =  0; y2 = th; x3 = tw; y3 = th; break;
                  case DIR_DOWN : x1 = tw / 2; y1 = th    ; x2 =  0; y2 =  0; x3 = tw; y3 =  0; break;
                  case DIR_LEFT : x1 =  0    ; y1 = th / 2; x2 = tw; y2 =  0; x3 = tw; y3 = th; break;
                  case DIR_RIGHT: x1 = tw    ; y1 = th / 2; x2 =  0; y2 =  0; x3 =  0; y3 = th; break;
                  default: throw fflerror("invalid direction: %s", pathf::dirName(d));
              }

              const auto xoff = (cw - tw) / 2; // move to center
              const auto yoff = (ch - th) / 2;

              g_sdlDevice->fillTriangle(Widget::evalU32(triangle.color, self),
                      dstDrawX + x1 + xoff, dstDrawY + y1 + yoff,
                      dstDrawX + x2 + xoff, dstDrawY + y2 + yoff,
                      dstDrawX + x3 + xoff, dstDrawY + y3 + yoff);

              switch(state){
                  case BEVENT_OFF : break;
                  case BEVENT_ON  : g_sdlDevice->fillRectangle(colorf::RED + colorf::A_SHF(32), dstDrawX, dstDrawY, cw, ch); break;
                  case BEVENT_DOWN: g_sdlDevice->fillRectangle(colorf::RED + colorf::A_SHF(96), dstDrawX, dstDrawY, cw, ch); break;
                  default: std::unreachable();
              }

              if(Widget::evalBool(frame.show, self)){
                  g_sdlDevice->drawRectangle(Widget::evalU32(frame.color, self), dstDrawX, dstDrawY, cw, ch);
              }
          },
      }}
{
    setGfxFunc([this]{ return &m_gfxDrawer; });
}

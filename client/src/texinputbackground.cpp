#include "pngtexdb.hpp"
#include "texinputbackground.hpp"

extern PNGTexDB *g_progUseDB;

TexInputBackground::TexInputBackground(TexInputBackground::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::move(args.w),
          .h = std::move(args.h),

          .parent = std::move(args.parent),
      }}

    , m_img
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000460); },

          .vflip = args.v,
          .rotate = args.v ? 1 : 0,
      }}

    , m_resize
      {{
          .getter = &m_img,
          .vr
          {
              3,
              3,
              m_img.w() - 6,
              m_img.h() - 6,
          },

          .resize = Widget::VarSize2D{[this] -> Widget::IntSize2D
          {
              return
              {
                  w() - 6,
                  h() - 6,
              };
          }},

          .parent{this},
      }}
{}

Widget::ROI TexInputBackground::gfxInputROI() const
{
    auto v = m_img.transposed();
    auto r = m_resize.gfxResizeROI();

    (v ? r.x : r.y) -= 1;
    (v ? r.w : r.h) += 2;

    return r;
}

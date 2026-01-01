#include "pngtexdb.hpp"
#include "texinputbackground.hpp"

extern PNGTexDB *g_progUseDB;

Widget::ROI TexInputBackground::fromInputROI(bool v, Widget::ROI r)
{
    r.x -= ( v ? 2 : 3);
    r.y -= (!v ? 2 : 3);

    r.w += ( v ? 4 : 6);
    r.h += (!v ? 4 : 6);

    return r;
}

Widget::IntSize2D TexInputBackground::borderSize(bool v)
{
    return
    {
         v ? 4 : 6,
        !v ? 4 : 6,
    };
}

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

Widget::ROI TexInputBackground::getInputROI() const
{
    auto r = m_resize.gfxResizeROI();

    (v() ? r.x : r.y) -= 1;
    (v() ? r.w : r.h) += 2;

    return r;
}

void TexInputBackground::setInputSize(Widget::VarSize2D argSize)
{
    setSize([this, argSize]{ return argSize.w(this) + ( v() ? 4 : 6); },
            [this, argSize]{ return argSize.h(this) + (!v() ? 4 : 6); });
}

void TexInputBackground::setInputSize(Widget::VarSize argW, Widget::VarSize argH)
{
    setSize([this, argW = std::move(argW)]{ return Widget::evalSize(argW, this) + ( v() ? 4 : 6); },
            [this, argH = std::move(argH)]{ return Widget::evalSize(argH, this) + (!v() ? 4 : 6); });
}

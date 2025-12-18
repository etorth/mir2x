#include "pngtexdb.hpp"
#include "texsliderbar.hpp"

extern PNGTexDB *g_progUseDB;

TexSliderBar::TexSliderBar(TexSliderBar::InitArgs args)
    : TexSlider
      {{
          .bar = std::move(args.bar),
          .index = args.index,
          .value = args.value,

          .onChange = std::move(args.onChange),
          .parent = std::move(args.parent),
      }}

    , m_imgSlot
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000460); },

          .vflip = vbar(),
          .rotate = vbar() ? 1 : 0,
      }}

    , m_imgBar
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000470); },

          .vflip = vbar(),
          .rotate = vbar() ? 1 : 0,

          .modColor = [this] -> uint32_t
          {
              if(active()){ return colorf::WHITE + colorf::A_SHF(0XFF); }
              else        { return colorf::GREY  + colorf::A_SHF(0XFF); }
          },
      }}

    , m_bg
      {{
          .w = std::nullopt,
          .h = std::nullopt,
      }}

    , m_slot
      {{
          .getter = &m_imgSlot,
          .vr
          {
              /* x */  vbar() ? 0 : 3,
              /* y */ !vbar() ? 0 : 3,
              /* w */  vbar() ? m_imgSlot.w() : m_imgSlot.w() - 6,
              /* h */ !vbar() ? m_imgSlot.h() : m_imgSlot.h() - 6,
          },

          .resize = Widget::VarSize2D{[this] -> Widget::IntSize2D
          {
              const auto roi = getBarROI(0, 0);
              return
              {
                  vbar() ? m_imgSlot.w() : roi.w,
                 !vbar() ? m_imgSlot.h() : roi.h,
              };
          }},

          .parent{&m_bg},
      }}

    , m_bar
      {{
          .x =  vbar() ? 2 : 3,
          .y = !vbar() ? 2 : 3,

          .w = [this]{ return  vbar() ? m_imgBar.w() : (getBarROI(0, 0).w  * getValue()); },
          .h = [this]{ return !vbar() ? m_imgBar.h() : (getBarROI(0, 0).h  * getValue()); },

          .getter = &m_imgBar,
          .parent{&m_bg},
      }}
{
    setBarBgWidget(m_bar.dx(), m_bar.dy(), &m_bg, false);
}

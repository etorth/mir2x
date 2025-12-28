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

    , m_bg
      {{
          .v = vbar(),
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

    , m_bar
      {{
          .x = [this]{ return m_bg.getInputROI().x; },
          .y = [this]{ return m_bg.getInputROI().y; },

          .w = [this]{ return m_bg.getInputROI().w * ( vbar() ? 1.0f : getValue()); },
          .h = [this]{ return m_bg.getInputROI().h * (!vbar() ? 1.0f : getValue()); },

          .getter = &m_imgBar,
          .parent{&m_bg},
      }}
{
    m_bg.setInputSize(Widget::VarSize2D([this]
    {
        return getBarROI(0, 0).size();
    }));

    setBarBgWidget(m_bar.dx(), m_bar.dy(), &m_bg, false);
}

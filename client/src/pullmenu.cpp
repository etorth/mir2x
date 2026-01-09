#include "pngtexdb.hpp"
#include "pullmenu.hpp"

extern PNGTexDB *g_progUseDB;
PullMenu::PullMenu(PullMenu::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .attrs
          {
              .inst
              {
                  .moveOnFocus = false,
              },
          },

          .parent = std::move(args.parent),
      }}

    , m_tips
      {{
          .label = args.label.text,
      }}

    , m_tipsCrop
      {{
          .getter = &m_tips,
          .vr
          {
              [wo = std::move(args.label.w), this]{ return Widget::evalSizeOpt(wo, this, [this]{ return m_tips.w(); }); },
              [ho = std::move(args.label.h), this]{ return Widget::evalSizeOpt(ho, this, [this]{ return m_tips.h(); }); },
          },
      }}

    , m_title
      {{
          .label = args.title.text,
      }}

    , m_titleBg
      {{
          .w = [wo = std::move(args.title.w), this]{ return Widget::evalSizeOpt(wo, this, [this]{ return m_title.w() + TexInputBackground::borderSize(false).w; }); },
          .h = [ho = std::move(args.title.h), this]{ return Widget::evalSizeOpt(ho, this, [this]{ return m_title.h() + TexInputBackground::borderSize(false).h; }); },
          .v = false,
      }}

    , m_imgOff {{.w = 22, .h = 22, .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000301); }, .rotate = 1}}
    , m_imgOn  {{.w = 22, .h = 22, .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000300); }, .rotate = 1}}
    , m_imgDown{{.w = 22, .h = 22, .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000302); }, .rotate = 1}}

    , m_button
      {{
          .gfxList
          {
              &m_imgOff,
              &m_imgOn,
              &m_imgDown,
          },

          .onTrigger = [this](int)
          {
              m_menuBoard.setFocus(!m_menuBoard.show());
              m_menuBoard.flipShow();
          },

          .attrs
          {
              .show = std::move(args.showButton),
          },
      }}

    , m_flex
      {{
          .v = false,
          .align = ItemAlign::CENTER,

          .itemSpace = 3,
          .childList
          {
              {&m_tipsCrop, false},
              {&m_titleBg , false},
              {&m_button  , false},
          },

          .parent{this},
      }}

    , m_menuBoard
      {{
          .fixed = std::move(args.menuFixed),
          .margin
          {
              5,
              5,
              5,
              5,
          },

          .corner = 3,
          .itemSpace = 6,
          .separatorSpace = 7,

          .itemList = std::move(args.itemList),
          .onClick  = std::move(args.onClick),
      }}

    , m_menuButton
      {{
          .x = [this]{ return m_titleBg.rdx(this) + m_titleBg.getInputROI().x; },
          .y = [this]{ return m_titleBg.rdy(this) + m_titleBg.getInputROI().y; },

          .itemSize
          {
              .w = [this]{ return m_titleBg.getInputROI().w; },
              .h = [this]{ return m_titleBg.getInputROI().h; },
          },

          .gfxWidget{&m_title},
          .subWidget{&m_menuBoard},

          .bgColor = colorf::GREY + colorf::A_SHF(64),
          .parent{this},
      }}
{}

void PullMenu::setFocus(bool argFocus)
{
    const auto oldFocus = focus();
    const auto newFocus = argFocus;

    Widget::setFocus(argFocus);

    if(oldFocus && !newFocus){
        m_menuBoard.setShow(false);
    }
}

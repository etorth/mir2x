#include "pngtexdb.hpp"
#include "pullmenu.hpp"

extern PNGTexDB *g_progUseDB;

PullMenu::PullMenu(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabel,
        int argLabelWidth,

        int argTitleBgWidth,
        int argTitleBgHeight,

        std::initializer_list<std::tuple<Widget *, bool, bool>> argMenuList,
        std::function<void(Widget *)> argOnClickMenu,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,
          .w = {},
          .h = {},

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_label
      {{
          .label = argLabel,
          .font
          {
              .id = 1,
              .size = 12,
          },
      }}

    , m_labelCrop
      {{
          .getter = &m_label,
          .vr
          {
              fflcheck(argLabelWidth, argLabelWidth >= 0),
              m_label.h(),
          },
          .parent{this},
      }}

    , m_menuTitleImage
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(0X00000460); },
      }}

    , m_menuTitleBackground
      {{
          .getter = &m_menuTitleImage,
          .vr
          {
              3,
              3,
              m_menuTitleImage.w() - 6,
              2,
          },

          .resize
          {
              fflcheck(argTitleBgWidth , argTitleBgWidth  >= 0),
              fflcheck(argTitleBgHeight, argTitleBgHeight >= 0),
          },

          .parent{this},
      }}

    , m_menuTitle
      {{
          .label = u8"NA",
          .font
          {
              .id = 1,
              .size = 12,
          },
      }}

    , m_menuTitleCrop
      {{
          .getter = &m_menuTitle,
          .vr
          {
              m_menuTitleBackground.w() - 6,
              std::min<int>(m_menuTitleBackground.h() - 4, m_menuTitle.h()),
          },

          .attrs
          {
              .inst
              {
                  .processEvent = [this](Widget *self, const SDL_Event &event, bool valid, Widget::ROIMap m)
                  {
                      if(!m.calibrate(self)){
                          return false;
                      }

                      if(!valid){
                          return self->consumeFocus(false);
                      }

                      switch(event.type){
                          case SDL_MOUSEBUTTONUP:
                              {
                                  if(m.in(SDLDeviceHelper::getEventPLoc(event).value())){
                                      m_menuList.setShow(true);
                                      return self->consumeFocus(true);
                                  }
                                  else{
                                      return false;
                                  }
                              }
                          default:
                              {
                                  return false;
                              }
                      }
                  },
              },
          },

          .parent{this},
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

          .onTrigger = [this](Widget *, int)
          {
              m_menuList.flipShow();
          },

          .parent{this},
      }}

    , m_menuList
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *)
          {
              return m_menuTitleCrop.w();
          },

          {5, 5, 5, 5},

          3,
          6,
          7,

          argMenuList,
          std::move(argOnClickMenu),

          this,
          false,
      }
{
    m_menuList.setShow(false);
    const int maxHeight = std::max<int>({m_labelCrop.h(), m_menuTitleBackground.h(), m_button.h()});

    m_labelCrop          .moveAt(DIR_LEFT, 0                                 , maxHeight / 2);
    m_menuTitleBackground.moveAt(DIR_LEFT, m_labelCrop.dx() + m_labelCrop.w(), maxHeight / 2);

    m_menuTitleCrop.moveAt(DIR_LEFT, m_menuTitleBackground.dx() + 3                        , maxHeight / 2);
    m_button       .moveAt(DIR_LEFT, m_menuTitleBackground.dx() + m_menuTitleBackground.w(), maxHeight / 2);

    m_menuList.moveAt(DIR_UPLEFT, m_menuTitleBackground.dx() + 3, m_menuTitleBackground.dy() + m_menuTitleBackground.h() - 2);
}

void PullMenu::drawDefault(Widget::ROIMap m) const
{
    for(const auto p:
    {
        static_cast<const Widget *>(&m_menuTitleBackground),
        static_cast<const Widget *>(&m_labelCrop),
        static_cast<const Widget *>(&m_menuTitleCrop),
        static_cast<const Widget *>(&m_button),
        static_cast<const Widget *>(&m_menuList),
    }){
        if(p->show()){
            drawChild(p, m);
        }
    }
}

bool PullMenu::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(Widget::processEventDefault(event, valid, m)){
        if(!focus()){
            if(!m_menuList.show()){
                consumeFocus(true, &m_button);
            }
            else{
                setFocus(true);
            }
        }
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m_menuList.show()){
                    if(!m.create(m_menuList.roi()).in(SDLDeviceHelper::getEventPLoc(event).value())){
                        m_menuList.setShow(false);
                    }
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}


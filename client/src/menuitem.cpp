#include "sdldevice.hpp"
#include "menuitem.hpp"

extern SDLDevice *g_sdlDevice;

MenuItem::MenuItem(MenuItem::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .attrs
          {
              .type
              {
                  .canSetSize = false,
              },
          },
          .parent = std::move(args.parent),
      }}

    , m_subWidget(args.subWidget.widget)
    , m_gfxWidgetCrop
      {{
          .w = args.itemSize.w.value_or([gfxWidget=fflcheck(args.gfxWidget.widget)]{ return gfxWidget->w(); }),
          .h = args.itemSize.h.value_or([gfxWidget=fflcheck(args.gfxWidget.widget)]{ return gfxWidget->h(); }),

          .childList
          {
              {
                  .widget = args.gfxWidget.widget,
                  .dir = DIR_LEFT,

                  .x = 0,
                  .y = [this]{ return m_gfxWidgetCrop.h() / 2; },

                  .autoDelete = args.gfxWidget.autoDelete,
              },
          },
      }}

    , m_indicator
      {{
           .w = MenuItem::INDICATOR_W,
           .h = MenuItem::INDICATOR_H,

           .drawFunc = [](int dstDrawX, int dstDrawY)
           {
               const int x1 = dstDrawX;
               const int y1 = dstDrawY;

               const int x2 = dstDrawX;
               const int y2 = dstDrawY + MenuItem::INDICATOR_H - 1;

               const int x3 = dstDrawX + MenuItem::INDICATOR_W - 1;
               const int y3 = dstDrawY + MenuItem::INDICATOR_H / 2;

               g_sdlDevice->fillTriangle(colorf::BLUE_A255, x1, y1, x2, y2, x3, y3);
           },
      }}

    , m_canvas
      {{
          .flex = std::nullopt,

          .v = false,
          .align = ItemAlign::CENTER,

          .first {&m_gfxWidgetCrop},
          .second{&m_indicator    },
      }}

    , m_wrapper
      {{
          .wrapped{&m_canvas},
          .margin = std::move(args.margin),
          .bgDrawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->fillRectangle(colorf::BLACK_A255, dstDrawX, dstDrawY, self->w(), self->h());
          },
      }}

    , m_button
      {{
          .gfxList
          {
              &m_wrapper,
              &m_wrapper,
              &m_wrapper,
          },

          .onOverIn  = [this]{ if(m_subWidget){ m_subWidget->setShow(true ); }},
          .onOverOut = [this]{ if(m_subWidget){ m_subWidget->setShow(false); }},

          .parent{this},
      }}
{}

void MenuItem::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(m_button.show()){
        drawChild(&m_button, m);
    }

    if(m_subWidget && m_subWidget->show()){
        drawChild(m_subWidget, m);
    }
}

bool MenuItem::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(m_button.show()){
        return m_button.processEventParent(event, valid, m);
    }

    if(m_subWidget && m_subWidget->show()){
        return m_subWidget->processEventParent(event, valid, m);
    }

    return false;
}

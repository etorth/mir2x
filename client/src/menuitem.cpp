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

          .childList
          {
              {
                  .widget = new TrigfxButton
                  {{
                      .onOverIn = [expandOn = args.expandOnHover, this]
                      {
                          if(m_subWidget && Widget::evalBool(expandOn, this)){
                              m_subWidget->setShow(true);
                          }
                      },

                      .onOverOut = [expandOn = args.expandOnHover, this]
                      {
                          if(m_subWidget && Widget::evalBool(expandOn, this)){
                              m_subWidget->setShow(false);
                          }
                      },

                      .onTrigger = [onClick = std::move(args.onClick), gfxWidget = args.gfxWidget.widget](int)
                      {
                          MenuItem::evalClickCBFunc(onClick, gfxWidget);
                      },
                  }},

                  .autoDelete = true,
              },

              {
                  .widget = args.subWidget.widget,
                  .autoDelete = args.subWidget.autoDelete,
              },
          },

          .attrs
          {
              .type
              {
                  .canSetSize  = false,
                  .canAddChild = false,
              },

              .inst
              {
                  .moveOnFocus = false,
              },
          },
          .parent = std::move(args.parent),
      }}

    , m_subWidget(args.subWidget.widget)
    , m_gfxButton(fflcheck(dynamic_cast<TrigfxButton *>(firstChild())))

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
           .w = [showInd = args.showIndicator, this]{ return Widget::evalBool(showInd, this) ? MenuItem::INDICATOR_W : 0; },
           .h = [showInd = args.showIndicator, this]{ return Widget::evalBool(showInd, this) ? MenuItem::INDICATOR_H : 0; },

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
          .bgDrawFunc = [bgColor = std::move(args.bgColor), showSep = std::move(args.showSeparator), this](int dstDrawX, int dstDrawY)
          {
              if(m_gfxButton->getState() != BEVENT_OFF){
                  g_sdlDevice->fillRectangle(Widget::evalU32(bgColor, this), dstDrawX, dstDrawY, w(), h());
              }

              if(Widget::evalBool(showSep, this)){
                  const int  width = w();
                  const int dwidth = width >= 4 ? 2 : 0;

                  const int lineX1 = dstDrawX             + dwidth;
                  const int lineX2 = dstDrawX + width - 1 - dwidth;

                  const int lineY = dstDrawY + h() - 1;
                  g_sdlDevice->drawLine(colorf::GREY + colorf::A_SHF(128), lineX1, lineY, lineX2, lineY);
              }
          },
      }}
{
    m_gfxButton->setGfxList(
    {
        &m_wrapper,
        &m_wrapper,
        &m_wrapper,
    });

    if(m_subWidget){
        m_subWidget->setShow(false);
        m_subWidget->moveAt(DIR_UPLEFT,
                [d = args.subWidget.dir, this]{ return m_gfxButton->dx() + Widget::xSizeOff(d, [this]{ return m_gfxButton->w() + 1; }); },
                [d = args.subWidget.dir, this]{ return m_gfxButton->dy() + Widget::ySizeOff(d, [this]{ return m_gfxButton->h() + 1; }); });
    }
}

void MenuItem::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(m_gfxButton->show()){
        drawChild(m_gfxButton, m);
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

    if(m_gfxButton->show()){
        if(m_gfxButton->processEventParent(event, valid, m)){
            return true;
        }
    }

    if(m_subWidget && m_subWidget->show()){
        return m_subWidget->processEventParent(event, valid, m);
    }

    return false;
}

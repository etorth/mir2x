#include "colorf.hpp"
#include "sdldevice.hpp"
#include "menuboard.hpp"

extern SDLDevice *g_sdlDevice;
MenuBoard::MenuBoard(MenuBoard::InitArgs args)
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
                  .widget = new MarginWrapper
                  {{
                      .margin = std::move(args.margin),
                      .bgDrawFunc = [corner = args.corner](const Widget *self, int dstDrawX, int dstDrawY)
                      {
                          const int w = self->w();
                          const int h = self->h();
                          const int c = Widget::evalSize(corner, self);

                          g_sdlDevice->fillRectangle(colorf::BLACK_A255, dstDrawX, dstDrawY, w, h, c);
                          g_sdlDevice->drawRectangle(colorf:: GREY_A255, dstDrawX, dstDrawY, w, h, c);
                      },
                  }},

                  .autoDelete = true,
              },
          },

          .attrs
          {
              .type
              {
                  .setSize  = false,
                  .addChild = false,
              },

              .inst = std::move(args.attrs),
          },

          .parent = std::move(args.parent),
      }}

    , m_itemSpace     (std::move(args.itemSpace))
    , m_separatorSpace(std::move(args.separatorSpace))

    , m_onClickMenu(std::move(args.onClick))
    , m_canvas
      {{
          .fixed = std::move(args.fixed),
      }}
{
    dynamic_cast<MarginWrapper *>(firstChild())->setWrapped(&m_canvas, false);
    for(auto &addItemArgs: args.itemList){
        addMenu(std::move(addItemArgs));
    }
}

void MenuBoard::addMenu(MenuBoard::AddItemArgs args)
{
    if(!args.gfxWidget.widget){
        return;
    }

    m_canvas.addItem(new MenuItem
    {{
        .margin
        {
            .up = [this](const Widget *self)
            {
                if(self == m_canvas.firstChild()){
                    return 0;
                }
                else{
                    return Widget::evalSize(m_itemSpace, this) / 2;
                }
            },

            .down = [addSep = std::move(args.showSeparator), this](const Widget *self)
            {
                if(self == m_canvas.lastChild()){
                    return 0;
                }
                else if(Widget::evalBool(addSep, self)){
                    return (Widget::evalSize(m_itemSpace, this) + 1) / 2 + Widget::evalSize(m_separatorSpace, this);
                }
                else{
                    return (Widget::evalSize(m_itemSpace, this) + 1) / 2;
                }
            },
        },

        .itemSize
        {
            // use gfxWidget.widget->w()
            // causes menu item width not aligned
            .w = std::nullopt,

            // don't use
            //
            //      .w = [this]{ return m_canvas.w(); },
            //
            // here, because m_canvas.w() is 0 at this moment
            // if add menu in this way, it causes all added MenuItem::itemSize as 0
        },

        .gfxWidget = std::move(args.gfxWidget),
        .subWidget
        {
            .dir = DIR_UPRIGHT,
            .widget = args.subWidget.widget,
            .autoDelete = args.subWidget.autoDelete,
        },

        .showIndicator = std::move(args.showIndicator),
        .showSeparator = [showSep = std::move(args.showSeparator), this](const Widget *self)
        {
            return Widget::evalBool(showSep, this) && (self != m_canvas.lastChild());
        },

        .expandOnHover = true,

        .bgColor = colorf::GREY + colorf::A_SHF(128),
        .onClick = [itemCB = m_onClickMenu, this](Widget *widget)
        {
            MenuItem::evalClickCBFunc(itemCB, widget); // widget is gfxWidget
            flipShow();
        },
    }},

    true);
}

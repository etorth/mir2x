#include "widget.hpp"
#include "menubutton.hpp"

MenuButton::MenuButton(MenuButton::InitArgs args)
    : MenuItem
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .margin = std::move(args.margin),
          .itemSize = std::move(args.itemSize),

          .gfxWidget = std::move(args.gfxWidget),
          .subWidget
          {
              .dir = DIR_DOWNLEFT,
              .widget = args.subWidget.widget,
              .autoDelete = args.subWidget.autoDelete,
          },

          .expandOnHover = std::move(args.expandOnHover),

          .bgColor = std::move(args.bgColor),
          .onClick = [this]
          {
              if(subWidget()){
                  subWidget()->setFocus(!subWidget()->show());
                  subWidget()->flipShow();
              }
          },

          .parent = std::move(args.parent),
      }}
{}

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
          .parent = std::move(args.parent),
      }}
{}

#include "widget.hpp"
#include "menubutton.hpp"

MenuButton::MenuButton(MenuButton::InitArgs args)
    : TrigfxButton
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .gfxList
          {
              args.gfxWidget.widget,
              args.gfxWidget.widget,
              args.gfxWidget.widget,
          },

          .onTrigger = [this](int)
          {
              if(m_menuBoard){
                  m_menuBoard->flipShow();
              }
          },

          .offXOnClick = 1,
          .offYOnClick = 1,
          .onClickDone = false,

          .parent = std::move(args.parent),
      }}

    , m_gfxWidget(args.gfxWidget.widget)
    , m_menuBoard(args.menuBoard.widget)
{
    if(m_gfxWidget) addChild(m_gfxWidget, args.gfxWidget.autoDelete); // for lifetime management only
    if(m_menuBoard) addChild(m_menuBoard, args.menuBoard.autoDelete); // ...
}

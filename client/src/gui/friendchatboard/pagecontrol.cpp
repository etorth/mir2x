#include "pagecontrol.hpp"
#include "friendchatboard.hpp"

PageControl::PageControl(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        int argSpace,

        std::initializer_list<std::pair<Widget *, bool>> argChildList,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          {},
          {},
          {},

          argParent,
          argAutoDelete,
      }
{
    int offX = 0;
    for(auto &[widgetPtr, autoDelete]: argChildList){
        addChild(widgetPtr, DIR_UPLEFT, offX, (h() - widgetPtr->h()) / 2, autoDelete);
        offX += widgetPtr->w();
        offX += argSpace;
    }
}

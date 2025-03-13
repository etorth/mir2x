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
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          {},
          {},
          {},

          argParent,
          argAutoDelete,
      }
{
    int maxH = 0;
    for(auto &[widgetPtr, autoDelete]: argChildList){
        maxH = std::max<int>(maxH, widgetPtr->h());
    }

    int offX = 0;
    for(auto &[widgetPtr, autoDelete]: argChildList){
        addChildAt(widgetPtr, DIR_UPLEFT, offX, (maxH - widgetPtr->h()) / 2, autoDelete);
        offX += widgetPtr->w();
        offX += argSpace;
    }
}

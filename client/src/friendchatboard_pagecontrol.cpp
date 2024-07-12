#include "friendchatboard.hpp"

FriendChatBoard::PageControl::PageControl(dir8_t argDir,

        int argX,
        int argY,

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
        addChild(widgetPtr, autoDelete);
        widgetPtr->moveAt(DIR_UPLEFT, offX, (h() - widgetPtr->h()) / 2);

        offX += widgetPtr->w();
        offX += argSpace;
    }
}

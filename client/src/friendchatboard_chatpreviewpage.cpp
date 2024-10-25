#include "friendchatboard.hpp"

FriendChatBoard::ChatPreviewPage::ChatPreviewPage(WidgetVarDir argDir,

        WidgetVarOffset argX,
        WidgetVarOffset argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          UIPage_WIDTH  - UIPage_MARGIN * 2,
          UIPage_HEIGHT - UIPage_MARGIN * 2,

          {},

          argParent,
          argAutoDelete,
      }

    , canvas
      {
          DIR_UPLEFT,
          0,
          0,

          this->w(),
          {},
          {},

          this,
          false,
      }
{}

void FriendChatBoard::ChatPreviewPage::updateChatPreview(const SDChatPeerID &sdCPID, const std::string &argMsg)
{
    ChatPreviewItem *child = dynamic_cast<ChatPreviewItem *>(canvas.hasChild([sdCPID](const Widget *widgetPtr, bool)
    {
        if(auto preview = dynamic_cast<const ChatPreviewItem *>(widgetPtr); preview && preview->cpid == sdCPID){
            return true;
        }
        return false;
    }));

    if(child){
        child->message.loadXML(argMsg.c_str());
    }
    else{
        child = new ChatPreviewItem
        {
            DIR_UPLEFT,
            0,
            0,

            sdCPID,
            to_u8cstr(argMsg),

            &canvas, // image load func uses getParentBoard(this)
            true,
        };
    }

    canvas.moveFront(child);

    int startY = 0;
    canvas.foreachChild([&startY](Widget *widget, bool)
    {
        widget->moveAt(DIR_UPLEFT, 0, startY);
        startY += widget->h();
    });
}

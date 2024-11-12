#include "chatpreviewitem.hpp"
#include "chatpreviewpage.hpp"
#include "friendchatboard.hpp"
#include "friendchatboardconst.hpp"

ChatPreviewPage::ChatPreviewPage(Widget::VarDir argDir,

        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,
        Widget::VarSize argH,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          std::move(argW),
          std::move(argH),

          {},

          argParent,
          argAutoDelete,
      }

    , canvas
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          {},
          {},

          this,
          false,
      }
{}

void ChatPreviewPage::updateChatPreview(const SDChatPeerID &sdCPID, const std::string &argMsg)
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
            [this](const Widget *){ return w(); },

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

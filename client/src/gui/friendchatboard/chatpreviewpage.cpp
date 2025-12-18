#include "chatpreviewitem.hpp"
#include "chatpreviewpage.hpp"
#include "friendchatboard.hpp"
#include "friendchatboardconst.hpp"

ChatPreviewPage::ChatPreviewPage(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarSizeOpt argW,
        Widget::VarSizeOpt argH,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = std::move(argW),
          .h = std::move(argH),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , canvas
      {{
          .w = [this]{ return w(); },
          .h = {},

          .parent
          {
              .widget = this,
          }
      }}
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

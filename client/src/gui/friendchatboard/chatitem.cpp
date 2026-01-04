#include "utf8f.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "chatitem.hpp"
#include "chatpage.hpp"
#include "friendchatboard.hpp"

extern SDLDevice *g_sdlDevice;

ChatItem::ChatItem(ChatItem::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .attrs
          {
              .type
              {
                  .setSize = false,
              },
          },

          .parent = std::move(args.parent),
      }}

    , pending(args.pending)
    , msgID  (args.msgID  )

    , showName  (args.showName  )
    , avatarLeft(args.avatarLeft)

    , bgColor(std::move(args.bgColor))

    , avatar
      {{
          .w = ChatItem::AVATAR_WIDTH,
          .h = ChatItem::AVATAR_HEIGHT,

          .texLoadFunc = std::move(args.texLoadFunc),
      }}

    , name
      {{
          .label = args.name,
          .font
          {
              .size = 10,
          },
      }}

    , message
      {{
          .lineWidth = std::max<int>(1, args.maxWidth - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN * 2),
          .initXML   = to_cstr(args.message),

          .onClickText = [this](const std::unordered_map<std::string, std::string> &attrList, int event)
          {
              if(event != BEVENT_RELEASE){
                  return;
              }

              const auto idstr = LayoutBoard::findAttrValue(attrList, "id");
              fflassert(idstr);

              const auto id = to_sv(idstr);
              if(id == SYS_AFRESP){
                  const auto cpidstr = LayoutBoard::findAttrValue(attrList, "cpid");
                  fflassert(cpidstr);

                  FriendChatBoard::getParentBoard(this)->queryChatPeer(SDChatPeerID(std::stoull(cpidstr)), [attrList, this](const SDChatPeer *sdCP, bool)
                  {
                      if(LayoutBoard::findAttrValue(attrList, "accept")){
                          FriendChatBoard::getParentBoard(this)->requestAcceptAddFriend(*sdCP);
                          if(LayoutBoard::findAttrValue(attrList, "addfriend")){
                              if(FriendChatBoard::getParentBoard(this)->findFriendChatPeer(sdCP->cpid())){
                                  FriendChatBoard::getParentBoard(this)->m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)"><t color="red">%s</t>已经是你的好友。</par>)###", to_cstr(sdCP->name));
                              }
                              else{
                                  FriendChatBoard::getParentBoard(this)->requestAddFriend(*sdCP, false);
                              }
                          }
                      }
                      else if(LayoutBoard::findAttrValue(attrList, "reject")){
                          FriendChatBoard::getParentBoard(this)->requestRejectAddFriend(*sdCP);
                          if(LayoutBoard::findAttrValue(attrList, "block")){
                              FriendChatBoard::getParentBoard(this)->requestBlockPlayer(*sdCP);
                          }
                      }
                  });
              }
          },
      }}

    , background
      {{
          .w = [this](const Widget *){ return ChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), ChatItem::MESSAGE_MIN_WIDTH ) + ChatItem::TRIANGLE_WIDTH; },
          .h = [this](const Widget *){ return ChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.h(), ChatItem::MESSAGE_MIN_HEIGHT)                           ; },

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
          {
              const uint32_t drawBGColor = Widget::evalU32Opt(bgColor, this, [this]
              {
                  if(avatarLeft){
                      return colorf::RED + colorf::A_SHF(128);
                  }
                  else if(pending){
                      return colorf::fadeRGBA(colorf::GREY + colorf::A_SHF(128), colorf::GREEN + colorf::A_SHF(128), std::fabs(std::fmod(accuTime / 1000.0, 2.0) - 1.0));
                  }
                  else{
                      return colorf::GREEN + colorf::A_SHF(128);
                  }
              });

              g_sdlDevice->fillRectangle(
                      drawBGColor,

                      drawDstX + (avatarLeft ? ChatItem::TRIANGLE_WIDTH : 0),
                      drawDstY,

                      std::max<int>(message.w(), ChatItem::MESSAGE_MIN_WIDTH ) + ChatItem::MESSAGE_MARGIN * 2,
                      std::max<int>(message.h(), ChatItem::MESSAGE_MIN_HEIGHT) + ChatItem::MESSAGE_MARGIN * 2,

                      ChatItem::MESSAGE_CORNER);

              const auto triangleX1_avatarLeft = drawDstX;
              const auto triangleX2_avatarLeft = drawDstX + ChatItem::TRIANGLE_WIDTH - 1;
              const auto triangleX3_avatarLeft = drawDstX + ChatItem::TRIANGLE_WIDTH - 1;

              const auto triangleX1_avatarRight = drawDstX + ChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), ChatItem::MESSAGE_MIN_WIDTH) + ChatItem::TRIANGLE_WIDTH - 1;
              const auto triangleX2_avatarRight = drawDstX + ChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), ChatItem::MESSAGE_MIN_WIDTH);
              const auto triangleX3_avatarRight = drawDstX + ChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), ChatItem::MESSAGE_MIN_WIDTH);

              const auto triangleY1_showName = drawDstY + (ChatItem::AVATAR_HEIGHT - ChatItem::NAME_HEIGHT) / 2;
              const auto triangleY2_showName = drawDstY + (ChatItem::AVATAR_HEIGHT - ChatItem::NAME_HEIGHT) / 2 - ChatItem::TRIANGLE_HEIGHT / 2;
              const auto triangleY3_showName = drawDstY + (ChatItem::AVATAR_HEIGHT - ChatItem::NAME_HEIGHT) / 2 + ChatItem::TRIANGLE_HEIGHT / 2;

              const auto triangleY1_hideName = drawDstY + ChatItem::AVATAR_HEIGHT / 2;
              const auto triangleY2_hideName = drawDstY + ChatItem::AVATAR_HEIGHT / 2 - ChatItem::TRIANGLE_HEIGHT / 2;
              const auto triangleY3_hideName = drawDstY + ChatItem::AVATAR_HEIGHT / 2 + ChatItem::TRIANGLE_HEIGHT / 2;

              if(avatarLeft){
                  if(showName) g_sdlDevice->fillTriangle(drawBGColor, triangleX1_avatarLeft, triangleY1_showName, triangleX2_avatarLeft, triangleY2_showName, triangleX3_avatarLeft, triangleY3_showName);
                  else         g_sdlDevice->fillTriangle(drawBGColor, triangleX1_avatarLeft, triangleY1_hideName, triangleX2_avatarLeft, triangleY2_hideName, triangleX3_avatarLeft, triangleY3_hideName);
              }
              else{
                  if(showName) g_sdlDevice->fillTriangle(drawBGColor, triangleX1_avatarRight, triangleY1_showName, triangleX2_avatarRight, triangleY2_showName, triangleX3_avatarRight, triangleY3_showName);
                  else         g_sdlDevice->fillTriangle(drawBGColor, triangleX1_avatarRight, triangleY1_hideName, triangleX2_avatarRight, triangleY2_hideName, triangleX3_avatarRight, triangleY3_hideName);
              }
          },
      }}

    , msgref(args.msgRefID.has_value() ? new ChatItemRef
      {
          DIR_UPLEFT,
          0,
          0,
          300,

          false,
          false,

          args.msgRefID.value(),
          to_cstr(args.messageRef),

      } : nullptr)
{
    if(avatarLeft){
        addChildAt(&avatar, DIR_UPLEFT, 0, 0, false);
        if(showName){
            addChildAt(&name      , DIR_LEFT  ,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH                           , ChatItem::NAME_HEIGHT / 2                       , false);
            addChildAt(&background, DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP                                                      , ChatItem::NAME_HEIGHT                           , false);
            addChildAt(&message   , DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH + ChatItem::MESSAGE_MARGIN, ChatItem::NAME_HEIGHT + ChatItem::MESSAGE_MARGIN, false);
        }
        else{
            addChildAt(&background, DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP                                                      , 0                                               , false);
            addChildAt(&message   , DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH + ChatItem::MESSAGE_MARGIN, ChatItem::MESSAGE_MARGIN                        , false);
        }
    }
    else{
        const auto fnRealWidth = [this]()
        {
            return ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH + std::max<int>
            ({
                showName ? name.w() : 0,
                std::max<int>(message.w(), ChatItem::MESSAGE_MIN_WIDTH) + ChatItem::MESSAGE_MARGIN * 2,
                msgref ? msgref->w() : 0,
            });
        };

        addChildAt(&avatar, DIR_UPRIGHT, [fnRealWidth](const Widget *){ return fnRealWidth() - 1; }, 0, false);
        if(showName){
            addChildAt(&name      , DIR_RIGHT  , [fnRealWidth](const Widget *){ return fnRealWidth() - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH                           ; }, ChatItem::NAME_HEIGHT / 2                       , false);
            addChildAt(&background, DIR_UPRIGHT, [fnRealWidth](const Widget *){ return fnRealWidth() - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP                                                      ; }, ChatItem::NAME_HEIGHT                           , false);
            addChildAt(&message   , DIR_UPRIGHT, [fnRealWidth](const Widget *){ return fnRealWidth() - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN; }, ChatItem::NAME_HEIGHT + ChatItem::MESSAGE_MARGIN, false);
        }
        else{
            addChildAt(&background, DIR_UPRIGHT, [fnRealWidth](const Widget *){ return fnRealWidth() - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP                                                      ; }, 0                                               , false);
            addChildAt(&message   , DIR_UPRIGHT, [fnRealWidth](const Widget *){ return fnRealWidth() - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN; }, ChatItem::MESSAGE_MARGIN                        , false);
        }
    }

    if(msgref){
        if(avatarLeft) addChildAt(msgref, DIR_UPLEFT , [this](const Widget *){ return message.dx()                   - ChatItem::MESSAGE_MARGIN; }, [this](const Widget *){ return message.dy() + message.h() - 1 + ChatItem::REF_GAP; }, true);
        else           addChildAt(msgref, DIR_UPRIGHT, [this](const Widget *){ return message.dx() + message.w() - 1 + ChatItem::MESSAGE_MARGIN; }, [this](const Widget *){ return message.dy() + message.h() - 1 + ChatItem::REF_GAP; }, true);
    }
}

void ChatItem::setMaxWidth(int argWidth)
{
    message.setLineWidth(argWidth - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN * 2);
}

void ChatItem::updateDefault(double fUpdateTime)
{
    accuTime += fUpdateTime;
}

bool ChatItem::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(true
            && event.type == SDL_MOUSEBUTTONUP
            && event.button.button == SDL_BUTTON_RIGHT
            && m.create(background.roi()).in(event.button.x, event.button.y)){

        if(auto chatPage = hasParent<ChatPage>()){
            if(chatPage->menu){
                chatPage->removeChild(chatPage->menu->id(), true);
                chatPage->menu = nullptr;
            }

            chatPage->addChildAt((chatPage->menu = new MenuBoard
            {{
                .fixed = 200,
                .margin
                {
                    5,
                    5,
                    5,
                    5,
                },

                .corner = 3,
                .itemSpace = 5,
                .separatorSpace = 6,

                .itemList
                {
                    {{new LabelBoard{{.label=u8"引用", .attrs{.data = std::make_any<std::string>("引用")}}}, true}},
                    {{new LabelBoard{{.label=u8"复制", .attrs{.data = std::make_any<std::string>("复制")}}}, true}},
                },

                .onClick = [this](Widget *item) // create new menu board whenever click a new chat item
                {
                    if(const auto op = std::any_cast<std::string>(item->data()); op == "引用"){
                        std::string textStr = message.getText();
                        fflassert(utf8f::valid(textStr));

                        if(const auto size = utf8::distance(textStr.begin(), textStr.end()); size > 50){
                            auto p = textStr.begin();
                            utf8::advance(p, 50, textStr.end());
                            textStr.resize(std::distance(textStr.begin(), p));
                            textStr.append("...");
                        }
                        hasParent<ChatPage>()->enableChatRef(msgID.value(), "<layout>" + xmlf::toParString("%s：%s", name.getText().c_str(), textStr.c_str()) + "</layout>");
                    }
                },
            }}),

            DIR_UPLEFT,
            event.button.x - (m.x - m.ro->x),
            event.button.y - (m.y - m.ro->y),
            true);

            chatPage->menu->setShow(true);
            chatPage->menu->setFocus(true);
        }

        setFocus(false);
        return true;
    }

    if(Widget::processEventDefault(event, valid, m)){
        if(!focus()){
            setFocus(true);
        }

        if(auto chatPage = hasParent<ChatPage>()){
            if(chatPage->menu){
                chatPage->removeChild(chatPage->menu->id(), true);
                chatPage->menu = nullptr;
            }
        }
        return true;
    }
    return false;
}

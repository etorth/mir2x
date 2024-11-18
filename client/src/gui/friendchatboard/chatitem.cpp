#include "sdldevice.hpp"
#include "processrun.hpp"
#include "chatitem.hpp"
#include "chatpage.hpp"
#include "friendchatboard.hpp"

extern SDLDevice *g_sdlDevice;

ChatItem::ChatItem(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        int  argMaxWidth,
        bool argPending,

        const char8_t *argNameStr,
        const char8_t *argMessageStr,
        const char8_t *argMessageRefStr,

        std::function<SDL_Texture *(const ImageBoard *)> argLoadImageFunc,

        bool argShowName,
        bool argAvatarLeft,
        std::optional<uint32_t> argBGColor,

        Widget *argParent,
        bool argAutoDelete)

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

    , pending(argPending)
    , showName(argShowName)
    , avatarLeft(argAvatarLeft)
    , bgColor(std::move(argBGColor))

    , avatar
      {
          DIR_UPLEFT,
          0,
          0,

          ChatItem::AVATAR_WIDTH,
          ChatItem::AVATAR_HEIGHT,

          std::move(argLoadImageFunc),
      }

    , name
      {
          DIR_UPLEFT,
          0,
          0,

          argNameStr,

          1,
          10,
      }

    , message
      {
          DIR_UPLEFT,
          0,
          0,

          std::max<int>(1, argMaxWidth - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN * 2),

          to_cstr(argMessageStr),
          0,

          {},
          false,
          false,
          false,
          false,

          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_LEFT,
          0,
          0,

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          nullptr,
          [this](const std::unordered_map<std::string, std::string> &attrList, int event)
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
      }

    , background
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return ChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), ChatItem::MESSAGE_MIN_WIDTH ) + ChatItem::TRIANGLE_WIDTH; },
          [this](const Widget *){ return ChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.h(), ChatItem::MESSAGE_MIN_HEIGHT)                           ; },

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              const uint32_t drawBGColor = bgColor.value_or([this]
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
              }());

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
      }

    , msgref(argMessageRefStr ? new ChatItemRef
      {
          DIR_UPLEFT,
          0,
          0,
          300,

          false,
          false,

          to_cstr(argMessageRefStr),
      } : nullptr)
{
    disableSetSize();

    if(avatarLeft){
        addChild(&avatar, DIR_UPLEFT, 0, 0, false);
        if(showName){
            addChild(&name      , DIR_LEFT  ,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH                           , ChatItem::NAME_HEIGHT / 2                       , false);
            addChild(&background, DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP                                                      , ChatItem::NAME_HEIGHT                           , false);
            addChild(&message   , DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH + ChatItem::MESSAGE_MARGIN, ChatItem::NAME_HEIGHT + ChatItem::MESSAGE_MARGIN, false);
        }
        else{
            addChild(&background, DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP                                                      , 0                                               , false);
            addChild(&message   , DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH + ChatItem::MESSAGE_MARGIN, ChatItem::MESSAGE_MARGIN                        , false);
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

        addChild(&avatar, DIR_UPRIGHT, [fnRealWidth](const Widget *){ return fnRealWidth() - 1; }, 0, false);
        if(showName){
            addChild(&name      , DIR_RIGHT  , [fnRealWidth](const Widget *){ return fnRealWidth() - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH                           ; }, ChatItem::NAME_HEIGHT / 2                       , false);
            addChild(&background, DIR_UPRIGHT, [fnRealWidth](const Widget *){ return fnRealWidth() - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP                                                      ; }, ChatItem::NAME_HEIGHT                           , false);
            addChild(&message   , DIR_UPRIGHT, [fnRealWidth](const Widget *){ return fnRealWidth() - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN; }, ChatItem::NAME_HEIGHT + ChatItem::MESSAGE_MARGIN, false);
        }
        else{
            addChild(&background, DIR_UPRIGHT, [fnRealWidth](const Widget *){ return fnRealWidth() - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP                                                      ; }, 0                                               , false);
            addChild(&message   , DIR_UPRIGHT, [fnRealWidth](const Widget *){ return fnRealWidth() - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN; }, ChatItem::MESSAGE_MARGIN                        , false);
        }
    }

    if(msgref){
        if(avatarLeft) addChild(msgref, DIR_UPLEFT , [this](const Widget *){ return message.dx()                   - ChatItem::MESSAGE_MARGIN; }, [this](const Widget *){ return message.dy() + message.h() - 1 + ChatItem::REF_GAP; }, true);
        else           addChild(msgref, DIR_UPRIGHT, [this](const Widget *){ return message.dx() + message.w() - 1 + ChatItem::MESSAGE_MARGIN; }, [this](const Widget *){ return message.dy() + message.h() - 1 + ChatItem::REF_GAP; }, true);
    }
}

void ChatItem::setMaxWidth(int argWidth)
{
    message.setLineWidth(argWidth - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN * 2);
}

void ChatItem::update(double fUpdateTime)
{
    accuTime += fUpdateTime;
}

bool ChatItem::processEventDefault(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(true
            && event.type == SDL_MOUSEBUTTONUP
            && event.button.button == SDL_BUTTON_RIGHT
            && background.in(event.button.x, event.button.y)){

        if(auto chatPage = dynamic_cast<ChatPage *>(parent(3))){
            if(chatPage->menu){
                chatPage->removeChild(chatPage->menu, true);
                chatPage->menu = nullptr;
            }

            chatPage->addChild((chatPage->menu = new MenuBoard
            {
                DIR_UPLEFT,
                0,
                0,
                200,

                {5, 5, 5, 5},

                3,
                5,
                6,

                {
                    {(new LabelBoard(DIR_UPLEFT, 0, 0, u8"引用" , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)))->setData(std::make_any<std::string>("引用")), false, true},
                    {(new LabelBoard(DIR_UPLEFT, 0, 0, u8"复制" , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)))->setData(std::make_any<std::string>("复制")), false, true},
                },

                [](Widget *item)
                {
                    if(const auto op = std::any_cast<std::string>(item->data()); op == "引用"){
                    }
                },
            }),

            DIR_UPLEFT,
            event.button.x - chatPage->x(),
            event.button.y - chatPage->y(),
            true);

            chatPage->menu->setShow(true);
            chatPage->menu->setFocus(true);
        }

        setFocus(false);
        return true;
    }

    if(Widget::processEventDefault(event, valid)){
        if(!focus()){
            setFocus(true);
        }

        if(auto chatPage = dynamic_cast<ChatPage *>(parent(3))){
            if(chatPage->menu){
                chatPage->removeChild(chatPage->menu, true);
                chatPage->menu = nullptr;
            }
        }
        return true;
    }
    return false;
}

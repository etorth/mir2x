#include "sdldevice.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern SDLDevice *g_sdlDevice;

FriendChatBoard::ChatItem::ChatItem(dir8_t argDir,
        int argX,
        int argY,

        bool argPending,

        const char8_t *argNameStr,
        const char8_t *argMessageStr,

        std::function<SDL_Texture *(const ImageBoard *)> argLoadImageFunc,

        bool argShowName,
        bool argAvatarLeft,
        std::optional<uint32_t> argBGColor,

        Widget *argParent,
        bool argAutoDelete)

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
          ChatItem::MAX_WIDTH - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN * 2,

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

                  getParentBoard(this)->queryChatPeer(SDChatPeerID(std::stoull(cpidstr)), [attrList, this](const SDChatPeer *sdCP, bool)
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

          ChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), ChatItem::MESSAGE_MIN_WIDTH ) + ChatItem::TRIANGLE_WIDTH,
          ChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.h(), ChatItem::MESSAGE_MIN_HEIGHT),

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

    , msgref
      {
          DIR_UPLEFT,
          0,
          0,
          300,

          false,
          false,

          to_cstr(argMessageStr),

          this,
          false,
      }
{
    const auto fnMoveAdd = [this](Widget *widgetPtr, dir8_t dstDir, int dstX, int dstY)
    {
        widgetPtr->moveAt(dstDir, dstX, dstY);
        addChild(widgetPtr, false);
    };

    if(avatarLeft){
        fnMoveAdd(&avatar, DIR_UPLEFT, 0, 0);
        if(showName){
            fnMoveAdd(&name      , DIR_LEFT  ,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH                                 , ChatItem::NAME_HEIGHT / 2                             );
            fnMoveAdd(&background, DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP                                                                  , ChatItem::NAME_HEIGHT                                 );
            fnMoveAdd(&message   , DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH + ChatItem::MESSAGE_MARGIN, ChatItem::NAME_HEIGHT + ChatItem::MESSAGE_MARGIN);
        }
        else{
            fnMoveAdd(&background, DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP                                                                  , 0                                                           );
            fnMoveAdd(&message   , DIR_UPLEFT,                  ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH + ChatItem::MESSAGE_MARGIN, ChatItem::MESSAGE_MARGIN                              );
        }
    }
    else{
        const auto realWidth = ChatItem::AVATAR_WIDTH + ChatItem::GAP + ChatItem::TRIANGLE_WIDTH + std::max<int>(message.w(), ChatItem::MESSAGE_MIN_WIDTH) + ChatItem::MESSAGE_MARGIN * 2;
        fnMoveAdd(&avatar, DIR_UPRIGHT, realWidth - 1, 0);

        if(showName){
            fnMoveAdd(&name      , DIR_RIGHT  , realWidth - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH                                 , ChatItem::NAME_HEIGHT / 2                             );
            fnMoveAdd(&background, DIR_UPRIGHT, realWidth - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP                                                                  , ChatItem::NAME_HEIGHT                                 );
            fnMoveAdd(&message   , DIR_UPRIGHT, realWidth - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN, ChatItem::NAME_HEIGHT + ChatItem::MESSAGE_MARGIN);
        }
        else{
            fnMoveAdd(&background, DIR_UPRIGHT, realWidth - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP                                                                  , 0                                                           );
            fnMoveAdd(&message   , DIR_UPRIGHT, realWidth - 1 - ChatItem::AVATAR_WIDTH - ChatItem::GAP - ChatItem::TRIANGLE_WIDTH - ChatItem::MESSAGE_MARGIN, ChatItem::MESSAGE_MARGIN                              );
        }
    }

    if(avatarLeft) fnMoveAdd(&msgref, DIR_UPLEFT , message.dx()                   - ChatItem::MESSAGE_MARGIN, message.dy() + message.h() - 1 + ChatItem::REF_GAP);
    else           fnMoveAdd(&msgref, DIR_UPRIGHT, message.dx() + message.w() - 1 + ChatItem::MESSAGE_MARGIN, message.dy() + message.h() - 1 + ChatItem::REF_GAP);

}

void FriendChatBoard::ChatItem::update(double fUpdateTime)
{
    accuTime += fUpdateTime;
}

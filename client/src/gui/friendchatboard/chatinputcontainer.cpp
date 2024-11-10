#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "chatpage.hpp"
#include "chatinputcontainer.hpp"
#include "friendchatboard.hpp"
#include "friendchatboardconst.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ChatInputContainer::ChatInputContainer(dir8_t argDir,

        int argX,
        int argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          UIPage_MIN_WIDTH - UIPage_MARGIN * 2 - ChatPage::INPUT_MARGIN * 2,
          [this](const Widget *)
          {
              return mathf::bound<int>(layout.h(), ChatPage::INPUT_MIN_HEIGHT, ChatPage::INPUT_MAX_HEIGHT);
          },

          {},

          argParent,
          argAutoDelete,
      }

    , layout
      {
          DIR_UPLEFT,
          0,
          0,

          this->w(),

          nullptr,
          0,

          {},
          false,
          true,
          true,
          false,

          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_JUSTIFY,
          0,
          0,

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          [this]()
          {
              if(!layout.hasToken()){
                  return;
              }

              auto message = layout.getXML();
              layout.clear();

              auto chatBoard = FriendChatBoard::getParentBoard(this);
              auto chatPage  = dynamic_cast<ChatPage *>(chatBoard->m_uiPageList[UIPage_CHAT].page);

              const SDChatMessage chatMessage
              {
                  .from  = chatBoard->m_processRun->getMyHero()->cpid(),
                  .to    = chatPage->peer.cpid(),
                  .message = cerealf::serialize(message),
              };

              chatPage->chat.append(chatMessage, [chatMessage, this](const ChatItem *chatItem)
              {
                  const uint64_t cpidu64 = chatMessage.to.asU64();

                  auto cpidsv  = as_sv(cpidu64);
                  auto msgbuf  = std::string();

                  msgbuf.append(cpidsv.begin(), cpidsv.end());
                  msgbuf.append(chatMessage.message.begin(), chatMessage.message.end());

                  const auto widgetID = chatItem->id();
                  const auto chatItemCanvas = std::addressof(dynamic_cast<ChatPage *>(parent())->chat.canvas);

                  FriendChatBoard::getParentBoard(this)->addMessagePending(widgetID, chatMessage);
                  g_client->send({CM_CHATMESSAGE, msgbuf}, [widgetID, chatItemCanvas, chatMessage, this](uint8_t headCode, const uint8_t *buf, size_t bufSize)
                  {
                      switch(headCode){
                          case SM_OK:
                              {
                                  const auto sdCMDBS = cerealf::deserialize<SDChatMessageDBSeq>(buf, bufSize);
                                  FriendChatBoard::getParentBoard(this)->finishMessagePending(widgetID, sdCMDBS);
                                  break;
                              }
                          default:
                              {
                                  throw fflerror("failed to send message");
                              }
                      }
                  });
              });
          },
          nullptr,

          this,
          false,
      }
{
    // there is mutual dependency
    // height of input container depends on height of layout
    //
    // layout always attach to buttom of input container, so argX needs container height
    // in initialization list we can not call this->h() since initialization of layout is not done yet
    layout.moveAt(DIR_DOWNLEFT, 0, [this](const Widget *){ return this->h() - 1; });
}

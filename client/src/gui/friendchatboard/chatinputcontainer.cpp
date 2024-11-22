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

ChatInputContainer::ChatInputContainer(
        Widget::VarDir  argDir,
        Widget::VarOff  argX,
        Widget::VarOff  argY,
        Widget::VarSize argW,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          std::move(argW),
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
                  .refer = chatPage->refopt(),
                  .from  = chatBoard->m_processRun->getMyHero()->cpid(),
                  .to    = chatPage->peer.cpid(),
                  .message = cerealf::serialize(message),
              };

              chatPage->disableChatRef();
              chatPage->chat.append(chatMessage, [chatMessage, this](const ChatItem *chatItem)
              {
                  CMChatMessageHeader cmCMH;
                  std::memset(&cmCMH, 0, sizeof(cmCMH));

                  cmCMH.toCPID = chatMessage.to.asU64();
                  cmCMH.hasRef = to_boolint(chatMessage.refer.has_value());
                  cmCMH.refID  = chatMessage.refer.value_or(0);

                  std::string msgbuf;
                  msgbuf = as_sv(cmCMH);
                  msgbuf.append(chatMessage.message.begin(), chatMessage.message.end());

                  const auto widgetID = chatItem->id();
                  FriendChatBoard::getParentBoard(this)->addMessagePending(widgetID, chatMessage);
                  g_client->send({CM_CHATMESSAGE, msgbuf}, [widgetID, this](uint8_t headCode, const uint8_t *buf, size_t bufSize)
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
    setAfterResize([this](Widget *)
    {
        layout.setLineWidth(this->w());
    });
}

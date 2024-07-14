#include "hero.hpp"
#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern PNGTexDB *g_progUseDB;

FriendChatBoard::ChatItemContainer::ChatItemContainer(dir8_t argDir,

        int argX,
        int argY,

        Widget::VarSize argH,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          UIPage_WIDTH - UIPage_MARGIN * 2,
          std::move(argH),

          {},

          argParent,
          argAutoDelete,
      }

    , canvas
      {
          DIR_UPLEFT,
          0,
          [this](const Widget *self)
          {
              if(self->h() < this->h()){
                  return 0;
              }

              return -1 * to_dround((self->h() - this->h()) * FriendChatBoard::getParentBoard(this)->m_uiPageList[UIPage_CHAT].slider->getValue());
          },

          this->w(),
          {},

          {},

          this,
          false,
      }
{}

void FriendChatBoard::ChatItemContainer::append(const SDChatMessage &sdCM, std::function<void(const FriendChatBoard::ChatItem *)> fnOp)
{
    auto chatItem = new ChatItem
    {
        DIR_UPLEFT,
        0,
        0,

        !sdCM.seq.has_value(),

        to_u8cstr("NA"),
        to_u8cstr(cerealf::deserialize<std::string>(sdCM.message)),

        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X010007CF);
        },

        sdCM.from != FriendChatBoard::getParentBoard(this)->m_processRun->getMyHeroChatPeer().cpid(),
        sdCM.from != FriendChatBoard::getParentBoard(this)->m_processRun->getMyHeroChatPeer().cpid(),

        {},
    };

    const auto startY = canvas.hasChild() ? (canvas.h() + ChatItem::ITEM_SPACE) : 0;

    if(chatItem->avatarLeft){
        chatItem->moveAt(DIR_UPLEFT, 0, startY);
    }
    else{
        chatItem->moveAt(DIR_UPRIGHT, canvas.w() - 1, startY);
    }

    canvas.addChild(chatItem, true);
    dynamic_cast<ChatPage *>(parent())->placeholder.setShow(false);

    FriendChatBoard::getParentBoard(this)->queryChatPeer(sdCM.from, [widgetID = chatItem->id(), sdCM, fnOp = std::move(fnOp), this](const SDChatPeer *peer, bool)
    {
        fflassert(peer, sdCM.from.asU64());
        if(auto chatItem = dynamic_cast<FriendChatBoard::ChatItem *>(this->canvas.hasChild(widgetID))){
            const auto from = sdCM.from;
            const auto job    = peer->player() ? peer->player()->job    : 0;
            const auto gender = peer->player() ? peer->player()->gender : false;

            chatItem->name.setText(to_u8cstr(peer->name));
            chatItem->avatar.setLoadFunc([from, job, gender](const ImageBoard *)
            {
                if     (from == SDChatPeerID(CP_SPECIAL, SYS_CHATDBID_SYSTEM)) return g_progUseDB->retrieve(0X00001100);
                else if(from == SDChatPeerID(CP_SPECIAL, SYS_CHATDBID_GROUP )) return g_progUseDB->retrieve(0X00001300);
                else                                                           return g_progUseDB->retrieve(Hero::faceGfxID(gender, job));
            });

            if(fnOp){
                fnOp(chatItem);
            }
        }
        else{
            if(fnOp){
                fnOp(nullptr);
            }
        }
    });
}

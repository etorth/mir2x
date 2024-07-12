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
    FriendChatBoard::getParentBoard(this)->queryChatPeer(sdCM.from, [sdCM, fnOp = std::move(fnOp), this](const SDChatPeer *peer, bool)
    {
        if(!peer){
            return;
        }

        const auto chatPage = dynamic_cast<ChatPage *>(parent());
        const auto self = FriendChatBoard::getParentBoard(this)->m_processRun->getMyHero();

        if(sdCM.from != chatPage->peer.cpid() && sdCM.to != chatPage->peer.cpid()){
            fnOp(nullptr);
            return;
        }

        auto chatItem = new ChatItem
        {
            DIR_UPLEFT,
            0,
            0,

            !sdCM.seq.has_value(),

            to_u8cstr(peer->name),
            to_u8cstr(cerealf::deserialize<std::string>(sdCM.message)),

            [from = sdCM.from, gender = peer->player() ? peer->player()->gender : false, job = peer->player() ? peer->player()->job : 0](const ImageBoard *)
            {
                if     (from == SDChatPeerID(CP_SPECIAL, SYS_CHATDBID_SYSTEM)) return g_progUseDB->retrieve(0X00001100);
                else if(from == SDChatPeerID(CP_SPECIAL, SYS_CHATDBID_GROUP )) return g_progUseDB->retrieve(0X00001300);
                else                                                           return g_progUseDB->retrieve(Hero::faceGfxID(gender, job));
            },

            peer->id != self->dbid(),
            peer->id != self->dbid(),

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
        if(fnOp){
            fnOp(chatItem);
        }

        dynamic_cast<ChatPage *>(parent())->placeholder.setShow(false);
    });
}

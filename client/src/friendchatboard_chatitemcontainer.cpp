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

    , nomsg
      {
          DIR_UP,
          canvas.w() / 2,
          ChatItem::ITEM_SPACE,

          u8"没有任何聊天记录，现在就开始聊天吧！",

          1,
          12,
          0,
          colorf::GREY + colorf::A_SHF(200),

          &canvas,
          false,
      }

    , ops
      {
          DIR_UPLEFT, // setup later
          0,
          0,

          300,

          "<layout><par>test</par></layout>",
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
          nullptr,

          &canvas,
          false,
      }
{
    ops.moveAt(DIR_UP, canvas.w() / 2, [this](const Widget *)
    {
        if(const auto lastItem = lastChatItem()){
            return lastItem->dy() + lastItem->h() + ChatItem::ITEM_SPACE;
        }
        else if(nomsg.show()){
            return nomsg.dy() + nomsg.h() + ChatItem::ITEM_SPACE;
        }
        else{
            return ChatItem::ITEM_SPACE;
        }
    });
}

void FriendChatBoard::ChatItemContainer::clearChatItem()
{
    canvas.clearChild([this](const Widget *child, bool)
    {
        return child != &nomsg && child != &ops;
    });

    nomsg.setShow(true);
    ops.setShow(false);
}

const FriendChatBoard::ChatItem *FriendChatBoard::ChatItemContainer::lastChatItem() const
{
    const Widget *lastItem = nullptr;
    canvas.foreachChild(false, [&lastItem, this](const Widget *widget, bool)
    {
        if(widget != &nomsg && widget != &ops){
            if(lastItem){
                if(lastItem->dy() + lastItem->h() < widget->dy() + widget->h()){
                    lastItem = widget;
                }
            }
            else{
                lastItem = widget;
            }
        }
        return false;
    });

    return dynamic_cast<const ChatItem *>(lastItem);
}

void FriendChatBoard::ChatItemContainer::append(const SDChatMessage &sdCM, std::function<void(const FriendChatBoard::ChatItem *)> fnOp)
{
    auto chatItem = new ChatItem
    {
        DIR_UPLEFT, // setup later
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

    const auto extraH = ChatItem::ITEM_SPACE + nomsg.h(); // extra height if nomsg box shows
    const auto startY = [extraH, this]() -> int // start Y if nomsg shows
    {
        if(const auto lastItem = lastChatItem()){
            if(nomsg.show()) return          lastItem->dy() + lastItem->h() + ChatItem::ITEM_SPACE;
            else             return extraH + lastItem->dy() + lastItem->h() + ChatItem::ITEM_SPACE;
        }
        else{
            return extraH + ChatItem::ITEM_SPACE;
        }
    }();

    if(chatItem->avatarLeft){
        chatItem->moveAt(DIR_UPLEFT, 0, [extraH, startY, this](const Widget *)
        {
            return startY - (nomsg.show() ? 0 : extraH);
        });
    }
    else{
        chatItem->moveAt(DIR_UPRIGHT, canvas.w() - 1, [extraH, startY, this](const Widget *)
        {
            return startY - (nomsg.show() ? 0 : extraH);
        });
    }

    nomsg.setShow(false);
    canvas.addChild(chatItem, true);

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

#include "hero.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

FriendChatBoard::ChatItemContainer::BackgroundWrapper::BackgroundWrapper(dir8_t argDir,
        int argX,
        int argY,

        int argMargin,
        int argCorner,

        Widget *argWidget, // widget should has been initialized

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          [argMargin, this](const Widget *){ return gfxWidget->w() + std::max<int>(0, argMargin) * 2; },
          [argMargin, this](const Widget *){ return gfxWidget->h() + std::max<int>(0, argMargin) * 2; },

          {
              {
                  argWidget,
                  DIR_NONE,

                  [this](const Widget *)
                  {
                      return w() / 2;
                  },

                  [this](const Widget *)
                  {
                      return h() / 2;
                  },

                  false,
              },
          },

          argParent,
          argAutoDelete,
      }

    , gfxWidget(argWidget)

    , background
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return this->w(); },
          [this](const Widget *){ return this->h(); },

          [argCorner](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, self->w(), self->h(), argCorner);
          },

          this,
          false,
      }
{
    moveFront(&background);
}

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
          DIR_UPLEFT, // ignored
          0,
          0,

          u8"没有任何聊天记录，现在就开始聊天吧！",

          1,
          12,
          0,
          colorf::GREY + colorf::A_SHF(200),
      }

    , ops
      {
          DIR_UPLEFT, // ignored
          0,
          0,

          300,

          "<layout><par>...</par></layout>",
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
              if(event == BEVENT_RELEASE){
                  if(const auto id = LayoutBoard::findAttrValue(attrList, "id", nullptr)){
                      if(to_sv(id) == "添加"){
                      }
                      else if(to_sv(id) == "屏蔽"){
                      }
                  }
              }
          },
      }

    , nomsgWrapper
      {
          DIR_UP,
          canvas.w() / 2,
          ChatItem::ITEM_SPACE,

          3,
          4,

          &nomsg,

          &canvas,
          false,
      }

    , opsWrapper
      {
          DIR_UPLEFT, // setup later
          0,
          0,

          3,
          4,

          &ops,

          &canvas,
          false,
      }
{
    opsWrapper.moveAt(DIR_UP, canvas.w() / 2, [this](const Widget *)
    {
        if(const auto lastItem = lastChatItem()){
            return lastItem->dy() + lastItem->h() + ChatItem::ITEM_SPACE;
        }
        else if(nomsgWrapper.show()){
            return nomsgWrapper.dy() + nomsgWrapper.h() + ChatItem::ITEM_SPACE;
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
        return child != &nomsgWrapper && child != &opsWrapper;
    });

    nomsgWrapper.setShow(true);
    opsWrapper.setShow(false);
}

const FriendChatBoard::ChatItem *FriendChatBoard::ChatItemContainer::lastChatItem() const
{
    const Widget *lastItem = nullptr;
    canvas.foreachChild([&lastItem, this](const Widget *widget, bool)
    {
        if(widget != &nomsgWrapper && widget != &opsWrapper){
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

        to_u8cstr("..."),
        to_u8cstr(cerealf::deserialize<std::string>(sdCM.message)),

        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X010007CF);
        },

        sdCM.from != FriendChatBoard::getParentBoard(this)->m_processRun->getMyHeroChatPeer().cpid(),
        sdCM.from != FriendChatBoard::getParentBoard(this)->m_processRun->getMyHeroChatPeer().cpid(),

        {},
    };

    const auto extraH = ChatItem::ITEM_SPACE + nomsgWrapper.h(); // extra height if nomsgWrapper box shows
    const auto startY = [extraH, this]() -> int // start Y if nomsgWrapper shows
    {
        if(const auto lastItem = lastChatItem()){
            if(nomsgWrapper.show()) return          lastItem->dy() + lastItem->h() + ChatItem::ITEM_SPACE;
            else                    return extraH + lastItem->dy() + lastItem->h() + ChatItem::ITEM_SPACE;
        }
        else{
            return extraH + ChatItem::ITEM_SPACE;
        }
    }();

    if(chatItem->avatarLeft){
        chatItem->moveAt(DIR_UPLEFT, 0, [extraH, startY, this](const Widget *)
        {
            return startY - (nomsgWrapper.show() ? 0 : extraH);
        });
    }
    else{
        chatItem->moveAt(DIR_UPRIGHT, canvas.w() - 1, [extraH, startY, this](const Widget *)
        {
            return startY - (nomsgWrapper.show() ? 0 : extraH);
        });
    }

    if(sdCM.from.group()){
        ops.loadXML(R"###(<layout><par>GROUP</par></layout>)###");
        opsWrapper.setShow(true);
    }
    else if(sdCM.from.player()){
        // if(findFriendChatPeer(sdCM.from)){
        //     opsWrapper.setShow(false);
        // }
        // else{
            ops.loadXML(R"###(<layout><par>对方不是你的好友，你可以<event id="添加">添加</event>对方为好友，或者<event id="屏蔽">屏蔽</event>对方的消息。</par></layout>)###");
            opsWrapper.setShow(true);
        // }
    }


    nomsgWrapper.setShow(false);
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

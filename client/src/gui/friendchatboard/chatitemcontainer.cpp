#include "hero.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "chatpage.hpp"
#include "chatitem.hpp"
#include "margincontainer.hpp"
#include "friendchatboard.hpp"
#include "chatitemcontainer.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ChatItemContainer::ChatItemContainer(
        Widget::VarDir  argDir,
        Widget::VarInt  argX,
        Widget::VarInt  argY,
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

          .attrs
          {
              .inst
              {
                  .afterResize = [this](Widget *)
                  {
                      canvas.foreachItem([this](Widget *chatItemBox, bool)
                      {
                          auto chatItemWidget = dynamic_cast<MarginContainer *>(chatItemBox)->contained();
                          auto chatItem       = dynamic_cast<ChatItem *>(chatItemWidget);

                          // can be nomsgBox or opsBox

                          if(chatItem){
                              chatItem->setMaxWidth(chatItemMaxWidth());
                          }
                      });

                      Widget::afterResizeDefault();
                  },
              },

          },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , canvas
      {{
          .y = [this](const Widget *self)
          {
              if(self->h() < this->h()){
                  return 0;
              }

              return -1 * to_dround((self->h() - this->h()) * FriendChatBoard::getParentBoard(this)->m_uiPageList[UIPage_CHAT].slider->getValue());
          },

          .fixed = [this]{ return w(); },
          .itemSpace = ChatItemContainer::ITEM_SPACE,

          .parent{this},
      }}

    , nomsg
      {{
          .label = u8"没有任何聊天记录，现在就开始聊天吧！",
          .font
          {
              .color = colorf::GREY_A255,
          },
      }}

    , ops
      {{
          .lineWidth = 300,
          .initXML = "<layout><par>...</par></layout>",

          .onClickText = [this](const std::unordered_map<std::string, std::string> &attrList, int event)
          {
              if(event == BEVENT_RELEASE){
                  if(const auto id = LayoutBoard::findAttrValue(attrList, "id", nullptr)){
                      if(to_sv(id) == "添加"){
                          FriendChatBoard::getParentBoard(this)->requestAddFriend(getChatPeer(), false);
                      }
                      else if(to_sv(id) == "屏蔽"){
                      }
                  }
              }
          },
      }}

    , nomsgBox
      {{
          .w = [this]{ return canvas.w(); },
          .h = [this]{ return nomsg .h() + 2 * ChatItemContainer::BACKGROUND_MARGIN; },

          .contained
          {
              .widget = std::addressof(nomsg),
          },

          .bgDrawFunc = [this](int startDstX, int startDstY)
          {
              const auto roi = nomsg.roi(this);
              g_sdlDevice->fillRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64),
                      startDstX + roi.x - ChatItemContainer::BACKGROUND_MARGIN,
                      startDstY + roi.y - ChatItemContainer::BACKGROUND_MARGIN,
                      roi.w + ChatItemContainer::BACKGROUND_MARGIN * 2,
                      roi.h + ChatItemContainer::BACKGROUND_MARGIN * 2, ChatItemContainer::BACKGROUND_CORNER);
          },
      }}

    , opsBox
      {{
          .w = [this]{ return canvas.w(); },
          .h = [this]{ return ops.h() + 2 * ChatItemContainer::BACKGROUND_MARGIN; },

          .contained
          {
              .widget = std::addressof(ops),
          },

          .bgDrawFunc = [this](const Widget *, int startDstX, int startDstY)
          {
              g_sdlDevice->fillRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64),
                      startDstX - ChatItemContainer::BACKGROUND_MARGIN,
                      startDstY - ChatItemContainer::BACKGROUND_MARGIN,
                      ops.w() + ChatItemContainer::BACKGROUND_MARGIN * 2,
                      ops.h() + ChatItemContainer::BACKGROUND_MARGIN * 2, ChatItemContainer::BACKGROUND_CORNER);
          },
      }}
{
    canvas.addItem(&nomsgBox, false);
}

int ChatItemContainer::chatItemMaxWidth() const
{
    return canvas.w() - ChatItem::TRIANGLE_WIDTH - ChatItem::GAP - ChatItem::AVATAR_WIDTH;
}

const SDChatPeer &ChatItemContainer::getChatPeer() const
{
    return hasParent<ChatPage>()->peer;
}

void ChatItemContainer::clearChatItem(bool keepNomsg)
{
    canvas.clearItem([keepNomsg, this](const Widget *item, bool)
    {
        if(keepNomsg){
            return item != &nomsgBox;
        }
        return true;
    });
}

void ChatItemContainer::append(const SDChatMessage &sdCM, std::function<void(const ChatItem *)> fnOp)
{
    auto chatItem = new ChatItem
    {{
        .maxWidth =  chatItemMaxWidth(), // cannot auto-stretch
        .pending  = !sdCM.seq.has_value(),

        .msgID    = sdCM.seq.has_value() ? std::make_optional(sdCM.seq.value().id) : std::nullopt,
        .msgRefID = sdCM.refer,

        .name = u8"...",
        .message = to_u8cstr(cerealf::deserialize<std::string>(sdCM.message)),
        .messageRef = sdCM.refer.has_value() ? u8"<layout><par>...</par></layout>" : nullptr,

        .texLoadFunc = []{ return g_progUseDB->retrieve(0X010007CF); },

        .showName   = sdCM.from != FriendChatBoard::getParentBoard(this)->m_processRun->getMyHeroChatPeer().cpid(),
        .avatarLeft = sdCM.from != FriendChatBoard::getParentBoard(this)->m_processRun->getMyHeroChatPeer().cpid(),
    }};

    auto chatItemBox = new MarginContainer
    {{
        .w = [this    ]{ return    canvas.w(); },
        .h = [chatItem]{ return chatItem->h(); },

        .contained
        {
            .dir = [chatItem](const Widget *)
            {
                return chatItem->avatarLeft ? DIR_LEFT : DIR_RIGHT;
            },

            .widget = chatItem,
            .autoDelete = true,
        },
    }};

    canvas.removeItem(nomsgBox.id(), false);
    canvas.removeItem(  opsBox.id(), false);

    canvas.addItem(chatItemBox, true);

    if(sdCM.from.group()){
        ops.loadXML(R"###(<layout><par>信息来源于群消息，请保护隐私。</par></layout>)###");
    }
    else if(sdCM.from.player()){
        ops.loadXML(R"###(<layout><par>对方不是你的好友，你可以<event id="添加">添加</event>对方为好友，或者<event id="屏蔽">屏蔽</event>对方的消息。</par></layout>)###");
    }

    hasParent<FriendChatBoard>()->queryChatPeer(sdCM.from, [widgetID = chatItem->id(), sdCM, fnOp = std::move(fnOp), this](const SDChatPeer *peer, bool)
    {
        fflassert(peer, sdCM.from.asU64());
        if(auto chatItem = dynamic_cast<ChatItem *>(canvas.hasDescendant(widgetID))){
            const auto from = sdCM.from;
            const auto job    = peer->player() ? peer->player()->job    : 0;
            const auto gender = peer->player() ? peer->player()->gender : false;

            chatItem->name.setText(to_u8cstr(peer->name));
            chatItem->avatar.setLoadFunc([from, job, gender](const Widget *)
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

    if(sdCM.refer.has_value()){
        hasParent<FriendChatBoard>()->queryChatMessage(sdCM.refer.value(), [widgetID = chatItem->id(), this](const SDChatMessage *refMsg, bool)
        {
            if(auto chatItem = dynamic_cast<ChatItem *>(canvas.hasDescendant(widgetID))){
                if(!refMsg){
                    chatItem->msgref->loadXML(R"###(<layout><par><t color="RED">引用的信息不存在或者已被删除</t></par></layout>)###");
                    return;
                }

                hasParent<FriendChatBoard>()->queryChatPeer(refMsg->from, [compMsg = refMsg->message, widgetID, this](const SDChatPeer *peer, bool)
                {
                    if(auto chatItem = dynamic_cast<ChatItem *>(hasDescendant(widgetID))){
                        const auto xmlStr = cerealf::deserialize<std::string>(compMsg);
                        tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);

                        if(xmlDoc.Parse(xmlStr.c_str()) != tinyxml2::XML_SUCCESS){
                            throw fflerror("tinyxml2::XMLDocument::Parse() failed: %s", xmlStr.c_str());
                        }

                        fflassert(xmlf::checkNodeName(xmlDoc.FirstChild(), "layout"));
                        fflassert(xmlf::checkNodeName(xmlDoc.FirstChild()->FirstChild(), "par"));

                        auto nameText = str_printf("%s：", peer ? peer->name.c_str() : "[未知]");
                        auto nameTextNode = xmlDoc.NewText(nameText.c_str());

                        xmlDoc.FirstChild()->FirstChild()->InsertFirstChild(nameTextNode);
                        chatItem->msgref->loadXML(xmlf::toString(&xmlDoc));
                    }
                });
            }
        });
    }

    const bool needOps = [this]
    {
        if(getChatPeer().special()){
            return false;
        }

        if(getChatPeer().group()){
            return false;
        }

        if(getChatPeer().player() && getChatPeer().id == FriendChatBoard::getParentBoard(this)->m_processRun->getMyHeroDBID()){
            return false;
        }

        return !FriendChatBoard::getParentBoard(this)->findFriendChatPeer(getChatPeer().cpid());
    }();

    if(needOps){
        canvas.addItem(&opsBox, false);
    }
}

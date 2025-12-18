#include "hero.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "chatpreviewitem.hpp"
#include "friendchatboard.hpp"
#include "friendchatboardconst.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ChatPreviewItem::ChatPreviewItem(
        Widget::VarDir  argDir,
        Widget::VarInt  argX,
        Widget::VarInt  argY,
        Widget::VarSizeOpt argW,

        const SDChatPeerID &argCPID,
        const char8_t *argChatXMLStr,

        Widget *argParent,
        bool   argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = std::move(argW),
          .h = ChatPreviewItem::HEIGHT,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , cpid(argCPID)

    , avatar
      {{
          .x = ChatPreviewItem::ITEM_MARGIN,
          .y = ChatPreviewItem::ITEM_MARGIN,

          .w = ChatPreviewItem::AVATAR_WIDTH,
          .h = ChatPreviewItem::HEIGHT - ChatPreviewItem::ITEM_MARGIN * 2,

          .texLoadFunc = [this](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X010007CF);
          },

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , name
      {{
          .dir = DIR_LEFT,
          .x = ChatPreviewItem::ITEM_MARGIN + ChatPreviewItem::AVATAR_WIDTH + ChatPreviewItem::GAP,
          .y = ChatPreviewItem::ITEM_MARGIN + ChatPreviewItem::NAME_HEIGHT / 2,

          .label = u8"未知用户",
          .font
          {
              .id = 1,
              .size = 14,
          },

          .parent{this},
      }}

    , message
      {{
          .initXML = to_cstr(argChatXMLStr),
          .parLimit = 1,

          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::GREY_A255,
          },
      }}

    , messageClip
      {{
          .x = ChatPreviewItem::ITEM_MARGIN + ChatPreviewItem::AVATAR_WIDTH + ChatPreviewItem::GAP,
          .y = ChatPreviewItem::ITEM_MARGIN + ChatPreviewItem::NAME_HEIGHT,

          .w = [this](const Widget *)
          {
              return w() - ChatPreviewItem::ITEM_MARGIN * 2 - ChatPreviewItem::AVATAR_WIDTH - ChatPreviewItem::GAP;
          },

          .h = ChatPreviewItem::HEIGHT - ChatPreviewItem::ITEM_MARGIN * 2 - ChatPreviewItem::NAME_HEIGHT,

          .childList
          {
              {&message, DIR_UPLEFT, 0, 0, false},
          },

          .parent
          {
              .widget = this,
          }
      }}

    , selected
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](int drawDstX, int drawDstY)
          {
              if(Widget::ROIMap{.x=drawDstX, .y=drawDstY, .ro{roi()}}.in(SDLDeviceHelper::getMousePLoc())){
                  g_sdlDevice->fillRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
              }
              else{
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(32), drawDstX, drawDstY, w(), h());
              }
          },

          .parent{this},
      }}
{
    FriendChatBoard::getParentBoard(this)->queryChatPeer(this->cpid, [canvas = parent(), widgetID = id(), this](const SDChatPeer *peer, bool)
    {
        if(!canvas->hasChild(widgetID)){
            return;
        }

        if(!peer){
            return;
        }

        this->name.setText(to_u8cstr(peer->name));
        this->avatar.setLoadFunc([dbid = peer->id, group = peer->group(), gender = peer->player() ? peer->player()->gender : false, job = peer->player() ? peer->player()->job : 0](const Widget *)
        {
            if     (group                      ) return g_progUseDB->retrieve(0X00001300);
            else if(dbid == SYS_CHATDBID_SYSTEM) return g_progUseDB->retrieve(0X00001100);
            else                                 return g_progUseDB->retrieve(Hero::faceGfxID(gender, job));
        });
    });
}

bool ChatPreviewItem::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m.in(event.button.x, event.button.y)){
                    FriendChatBoard::getParentBoard(this)->m_processRun->requestLatestChatMessage({this->cpid.asU64()}, 50, true, true);
                    FriendChatBoard::getParentBoard(this)->queryChatPeer(this->cpid, [canvas = this->parent(), widgetID = this->id(), this](const SDChatPeer *peer, bool)
                    {
                        if(!peer){
                            return;
                        }

                        if(!canvas->hasChild(widgetID)){
                            return;
                        }

                        auto boardPtr = FriendChatBoard::getParentBoard(this);

                        boardPtr->setChatPeer(*peer, true);
                        boardPtr->setUIPage(UIPage_CHAT);
                    });
                    return consumeFocus(true);
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}

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
        Widget::VarOff  argX,
        Widget::VarOff  argY,
        Widget::VarSize argW,

        const SDChatPeerID &argCPID,
        const char8_t *argChatXMLStr,

        Widget *argParent,
        bool   argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          std::move(argW),
          ChatPreviewItem::HEIGHT,

          {},

          argParent,
          argAutoDelete,
      }

    , cpid(argCPID)

    , avatar
      {
          DIR_UPLEFT,
          ChatPreviewItem::ITEM_MARGIN,
          ChatPreviewItem::ITEM_MARGIN,

          ChatPreviewItem::AVATAR_WIDTH,
          ChatPreviewItem::HEIGHT - ChatPreviewItem::ITEM_MARGIN * 2,

          [this](const ImageBoard *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X010007CF);
          },

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(0XFF),

          this,
          false,
      }

    , name
      {
          DIR_LEFT,
          ChatPreviewItem::ITEM_MARGIN + ChatPreviewItem::AVATAR_WIDTH + ChatPreviewItem::GAP,
          ChatPreviewItem::ITEM_MARGIN + ChatPreviewItem::NAME_HEIGHT / 2,

          u8"未知用户",

          1,
          14,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }

    , message
      {
          DIR_UPLEFT,
          0,
          0,
          0, // line width

          to_cstr(argChatXMLStr),
          1,

          {},
          false,
          false,
          false,
          false,

          1,
          12,
          0,
          colorf::GREY + colorf::A_SHF(255),
      }

    , messageClip
      {
          DIR_UPLEFT,
          ChatPreviewItem::ITEM_MARGIN + ChatPreviewItem::AVATAR_WIDTH + ChatPreviewItem::GAP,
          ChatPreviewItem::ITEM_MARGIN + ChatPreviewItem::NAME_HEIGHT,

          [this](const Widget *)
          {
              return w() - ChatPreviewItem::ITEM_MARGIN * 2 - ChatPreviewItem::AVATAR_WIDTH - ChatPreviewItem::GAP;
          },

          ChatPreviewItem::HEIGHT - ChatPreviewItem::ITEM_MARGIN * 2 - ChatPreviewItem::NAME_HEIGHT,

          {
              {&message, DIR_UPLEFT, 0, 0, false},
          },

          this,
          false,
      }

    , selected
      {
          DIR_UPLEFT,
          0,
          0,

          this->w(),
          this->h(),

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              if(const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc(); in(mousePX, mousePY)){
                  g_sdlDevice->fillRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
              }
              else{
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(32), drawDstX, drawDstY, w(), h());
              }
          },

          this,
          false,
      }
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
        this->avatar.setLoadFunc([dbid = peer->id, group = peer->group(), gender = peer->player() ? peer->player()->gender : false, job = peer->player() ? peer->player()->job : 0](const ImageBoard *)
        {
            if     (group                      ) return g_progUseDB->retrieve(0X00001300);
            else if(dbid == SYS_CHATDBID_SYSTEM) return g_progUseDB->retrieve(0X00001100);
            else                                 return g_progUseDB->retrieve(Hero::faceGfxID(gender, job));
        });
    });
}

bool ChatPreviewItem::processEventDefault(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y)){
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

#include "hero.hpp"
#include "pngtexdb.hpp"
#include "frienditem.hpp"
#include "friendlistpage.hpp"
#include "friendchatboard.hpp"
#include "friendchatboardconst.hpp"

extern PNGTexDB *g_progUseDB;

FriendListPage::FriendListPage(Widget::VarDir argDir,

        Widget::VarInt argX,
        Widget::VarInt argY,

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

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , canvas
      {{
          .w = [this](const Widget *){ return w(); },
          .h = {},

          .parent
          {
              .widget = this,
          }
      }}
{}

void FriendListPage::append(const SDChatPeer &peer, std::function<void(FriendItem *)> argOnClick, std::pair<Widget *, bool> argFuncWidget)
{
    canvas.addChildAt(new FriendItem
    {
        DIR_UPLEFT,
        0,
        0,
        [this](const Widget *){ return w(); }, // use FriendListPage::w()

        SDChatPeerID(CP_PLAYER, peer.id),
        to_u8cstr(peer.name),

        [peer](const Widget *)
        {
            if     (peer.group()                  ) return g_progUseDB->retrieve(0X00001300);
            else if(peer.id == SYS_CHATDBID_SYSTEM) return g_progUseDB->retrieve(0X00001100);
            else                                    return g_progUseDB->retrieve(Hero::faceGfxID(peer.player()->gender, peer.player()->job));
        },

        std::move(argOnClick),
        std::move(argFuncWidget),
    },

    DIR_UPLEFT, 0, canvas.h(), true);
}

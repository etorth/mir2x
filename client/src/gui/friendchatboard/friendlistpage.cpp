#include "hero.hpp"
#include "pngtexdb.hpp"
#include "frienditem.hpp"
#include "friendlistpage.hpp"
#include "friendchatboard.hpp"
#include "friendchatboardconst.hpp"

extern PNGTexDB *g_progUseDB;

FriendListPage::FriendListPage(Widget::VarDir argDir,

        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,
        Widget::VarSize argH,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          std::move(argW),
          std::move(argH),

          {},

          argParent,
          argAutoDelete,
      }

    , canvas
      {
          DIR_UPLEFT,
          0,
          0,

          this->w(),
          {},
          {},

          this,
          false,
      }
{}

void FriendListPage::append(const SDChatPeer &peer, std::function<void(FriendItem *)> argOnClick, std::pair<Widget *, bool> argFuncWidget)
{
    canvas.addChild(new FriendItem
    {
        DIR_UPLEFT,
        0,
        0,

        SDChatPeerID(CP_PLAYER, peer.id),
        to_u8cstr(peer.name),

        [peer](const ImageBoard *)
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

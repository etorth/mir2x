#include "hero.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

FriendChatBoard::ChatPreviewItem::ChatPreviewItem(dir8_t argDir,
        int argX,
        int argY,

        const SDChatPeerID &argCPID,
        const char8_t *argChatXMLStr,

        Widget *argParent,
        bool   argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          ChatPreviewItem::WIDTH,
          ChatPreviewItem::HEIGHT,

          {},

          argParent,
          argAutoDelete,
      }

    , cpid(argCPID)

    , avatar
      {
          DIR_UPLEFT,
          0,
          0,

          ChatPreviewItem::AVATAR_WIDTH,
          ChatPreviewItem::HEIGHT,

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
          ChatPreviewItem::AVATAR_WIDTH + ChatPreviewItem::GAP,
          ChatPreviewItem::NAME_HEIGHT / 2,

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

    , preview
      {
          DIR_UPLEFT,
          ChatPreviewItem::AVATAR_WIDTH + ChatPreviewItem::GAP,
          ChatPreviewItem::NAME_HEIGHT,

          ChatPreviewItem::WIDTH - ChatPreviewItem::AVATAR_WIDTH - ChatPreviewItem::GAP,
          ChatPreviewItem::HEIGHT - ChatPreviewItem::NAME_HEIGHT,

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              message.drawEx(drawDstX, drawDstY, 0, 0, message.w(), message.h());
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

bool FriendChatBoard::ChatPreviewItem::processEvent(const SDL_Event &event, bool valid)
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
                        boardPtr->setUIPage(FriendChatBoard::UIPage_CHAT);
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

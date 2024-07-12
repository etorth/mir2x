#include "hero.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

FriendChatBoard::SearchPage::SearchPage(Widget::VarDir argDir,

        Widget::VarOffset argX,
        Widget::VarOffset argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          UIPage_WIDTH  - UIPage_MARGIN * 2,
          UIPage_HEIGHT - UIPage_MARGIN * 2,

          {},

          argParent,
          argAutoDelete,
      }

    , input
      {
          DIR_UPLEFT,
          0,
          0,

          this,
          false,
      }

    , clear
      {
          DIR_LEFT,
          input.dx() + input.w() + SearchPage::CLEAR_GAP,
          input.dy() + input.h() / 2,

          -1,

          R"###(<layout><par><event id="clear">清空</event></par></layout>)###",
          0,

          {},

          false,
          false,
          false,
          false,

          1,
          15,

          0,
          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_LEFT,
          0,
          0,

          0,
          0,

          nullptr,
          nullptr,
          [this](const std::unordered_map<std::string, std::string> &attrList, int event)
          {
              if(event == BEVENT_RELEASE){
                  if(const auto id = LayoutBoard::findAttrValue(attrList, "id", nullptr)){
                      input.input.clear();
                  }
              }
          },

          this,
          false,
      }

    , autocompletes
      {
          DIR_UPLEFT,
          0,
          SearchInputLine::HEIGHT,

          SearchPage::WIDTH,
          SearchPage::HEIGHT - SearchInputLine::HEIGHT,

          {},

          this,
          false,
      }

    , candidates
      {
          DIR_UPLEFT,
          0,
          SearchInputLine::HEIGHT,

          SearchPage::WIDTH,
          SearchPage::HEIGHT - SearchInputLine::HEIGHT,

          {},

          this,
          false,
      }
{
    candidates.setShow(false);
}

void FriendChatBoard::SearchPage::appendFriendItem(const SDChatPeer &candidate)
{
    int maxY = 0;
    candidates.foreachChild([&maxY](const Widget *widget, bool)
    {
        maxY = std::max<int>(maxY, widget->dy() + widget->h());
    });

    candidates.addChild(new FriendItem
    {
        DIR_UPLEFT,
        0,
        maxY,

        SDChatPeerID(CP_PLAYER, candidate.id),
        to_u8cstr(candidate.name),

        [gender = candidate.player()->gender, job = candidate.player()->job](const ImageBoard *)
        {
            return g_progUseDB->retrieve(Hero::faceGfxID(gender, job));
        },

        nullptr,

        {
            (candidate.id == FriendChatBoard::getParentBoard(this)->m_processRun->getMyHero()->dbid()) ? nullptr : new LayoutBoard
            {
                DIR_UPLEFT,
                0,
                0,
                0,

                R"###(<layout><par><event id="add">添加</event></par></layout>)###",
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

                0,
                0,

                nullptr,
                nullptr,
                [candidate, this](const std::unordered_map<std::string, std::string> &attrList, int event)
                {
                    if(event == BEVENT_PRESS){
                        if(const auto id = LayoutBoard::findAttrValue(attrList, "id"); to_sv(id) == "add"){
                            CMAddFriend cmAF;
                            std::memset(&cmAF, 0, sizeof(cmAF));

                            cmAF.dbid = candidate.id;
                            g_client->send({CM_ADDFRIEND, cmAF}, [candidate, this](uint8_t headCode, const uint8_t *buf, size_t bufSize)
                            {
                                switch(headCode){
                                    case SM_OK:
                                        {
                                            switch(const auto sdAFN = cerealf::deserialize<SDAddFriendNotif>(buf, bufSize); sdAFN.notif){
                                                case AF_ACCEPTED:
                                                    {
                                                        auto boardPtr = FriendChatBoard::getParentBoard(this);
                                                        boardPtr->m_sdFriendList.push_back(candidate);

                                                        dynamic_cast<FriendListPage  *>(boardPtr->m_uiPageList[UIPage_FRIENDLIST ].page)->append(candidate);
                                                        dynamic_cast<ChatPreviewPage *>(boardPtr->m_uiPageList[UIPage_CHATPREVIEW].page)->updateChatPreview(candidate.cpid(), str_printf(R"###(<layout><par><t color="red">%s</t>已经通过你的好友申请，现在可以开始聊天了。</par></layout>)###", to_cstr(candidate.name)));

                                                        boardPtr->setUIPage(UIPage_CHATPREVIEW);
                                                        break;
                                                    }
                                                case AF_EXIST:
                                                    {
                                                        FriendChatBoard::getParentBoard(this)->m_processRun->addCBLog(CBLOG_ERR, u8"重复添加好友%s", to_cstr(candidate.name));
                                                        break;
                                                    }
                                                default:
                                                    {
                                                        break;
                                                    }
                                            }
                                            break;
                                        }
                                    default:
                                        {
                                            throw fflerror("failed to add friend: %s", to_cstr(candidate.name));
                                        }
                                }
                            });
                        }
                    }
                },
            },

            true,
        },

    }, true);
}

void FriendChatBoard::SearchPage::appendAutoCompletionItem(bool byID, const SDChatPeer &candidate, const std::string &xmlStr)
{
    int maxY = 0;
    autocompletes.foreachChild([&maxY](const Widget *widget, bool)
    {
        maxY = std::max<int>(maxY, widget->dy() + widget->h());
    });

    autocompletes.addChild(new SearchAutoCompletionItem
    {
        DIR_UPLEFT,
        0,
        maxY,

        byID,
        candidate,
        xmlStr.c_str(),

    }, true);
}

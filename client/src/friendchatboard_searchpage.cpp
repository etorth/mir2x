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
                            FriendChatBoard::getParentBoard(this)->requestAddFriend(candidate);
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

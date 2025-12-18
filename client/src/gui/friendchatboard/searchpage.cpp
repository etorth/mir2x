#include "hero.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "searchpage.hpp"
#include "frienditem.hpp"
#include "searchautocompletionitem.hpp"
#include "friendchatboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

SearchPage::SearchPage(Widget::VarDir argDir,

        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .w = UIPage_MIN_WIDTH  - UIPage_MARGIN * 2,
          .h = UIPage_MIN_HEIGHT - UIPage_MARGIN * 2,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , input
      {
          DIR_UPLEFT,
          0,
          0,

          this,
          false,
      }

    , clear
      {{
          .dir = DIR_LEFT,
          .x = input.dx() + input.w() + SearchPage::CLEAR_GAP,
          .y = input.dy() + input.h() / 2,

          .initXML = R"###(<layout><par><event id="clear">清空</event></par></layout>)###",
          .font
          {
              .id = 1,
              .size = 15,
          },

          .onClickText = [this](const std::unordered_map<std::string, std::string> &attrList, int event)
          {
              if(event == BEVENT_RELEASE){
                  if(const auto id = LayoutBoard::findAttrValue(attrList, "id", nullptr)){
                      input.input.clear();
                  }
              }
          },

          .parent{this},
      }}

    , autocompletes
      {{
          .y = SearchInputLine::HEIGHT,

          .w = SearchPage::WIDTH,
          .h = SearchPage::HEIGHT - SearchInputLine::HEIGHT,

          .parent
          {
              .widget = this,
          }
      }}

    , candidates
      {{
          .y = SearchInputLine::HEIGHT,

          .w = SearchPage::WIDTH,
          .h = SearchPage::HEIGHT - SearchInputLine::HEIGHT,

          .parent
          {
              .widget = this,
          }
      }}
{
    candidates.setShow(false);
}

void SearchPage::appendFriendItem(const SDChatPeer &candidate)
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
        [this](const Widget *){ return w(); }, // use SearchPage::w()

        SDChatPeerID(CP_PLAYER, candidate.id),
        to_u8cstr(candidate.name),

        [gender = candidate.player()->gender, job = candidate.player()->job](const Widget *)
        {
            return g_progUseDB->retrieve(Hero::faceGfxID(gender, job));
        },

        nullptr,

        {
            (candidate.id == FriendChatBoard::getParentBoard(this)->m_processRun->getMyHero()->dbid()) ? nullptr : new LayoutBoard
            {{
                .initXML = R"###(<layout><par><event id="add">添加</event></par></layout>)###",
                .font
                {
                    .id = 1,
                    .size = 12,
                },

                .onClickText= [candidate, this](const std::unordered_map<std::string, std::string> &attrList, int event)
                {
                    if(event == BEVENT_PRESS){
                        if(const auto id = LayoutBoard::findAttrValue(attrList, "id"); to_sv(id) == "add"){
                            FriendChatBoard::getParentBoard(this)->requestAddFriend(candidate, true);
                        }
                    }
                },
            }},

            true,
        },
    }, true);
}

void SearchPage::appendAutoCompletionItem(bool byID, const SDChatPeer &candidate, const std::string &xmlStr)
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

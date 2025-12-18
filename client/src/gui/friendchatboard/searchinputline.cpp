#include "clientmsg.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "searchpage.hpp"
#include "searchinputline.hpp"
#include "friendchatboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;

SearchInputLine::SearchInputLine(Widget::VarDir argDir,

        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .w = SearchInputLine::WIDTH,
          .h = SearchInputLine::HEIGHT,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , image
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(0X00000460); },
      }}

    , inputbg
      {{
          .getter = &image,
          .vr
          {
              3,
              3,
              image.w() - 6,
              2,
          },

          .resize
          {
              [this]{ return w() - 6; },
              [this]{ return h() - (image.h() - 2); },
          },

          .parent{this},
      }}

    , icon
      {{
          .dir = DIR_NONE,
          .x = SearchInputLine::ICON_WIDTH / 2 + SearchInputLine::ICON_MARGIN + 3,
          .y = SearchInputLine::HEIGHT     / 2,

          .w = std::min<int>(SearchInputLine::ICON_WIDTH, SearchInputLine::HEIGHT - 3 * 2),
          .h = std::min<int>(SearchInputLine::ICON_WIDTH, SearchInputLine::HEIGHT - 3 * 2),

          .texLoadFunc = [](const Widget *) { return g_progUseDB->retrieve(0X00001200); },

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , input
      {
          DIR_UPLEFT,
          3 + SearchInputLine::ICON_MARGIN + SearchInputLine::ICON_WIDTH + SearchInputLine::GAP,
          3,

          SearchInputLine::WIDTH  - 3 * 2 - SearchInputLine::ICON_MARGIN - SearchInputLine::ICON_WIDTH - SearchInputLine::GAP,
          SearchInputLine::HEIGHT - 3 * 2,

          true,

          1,
          14,

          0,
          colorf::WHITE_A255,

          2,
          colorf::WHITE_A255,

          nullptr,
          [this]()
          {
              hasParent<SearchPage>()->candidates.setShow(true);
              hasParent<SearchPage>()->autocompletes.setShow(false);
          },

          [this](std::string query)
          {
              hint.setShow(query.empty());

              if(query.empty()){
                  hasParent<SearchPage>()->candidates.clearChild();
                  hasParent<SearchPage>()->autocompletes.clearChild();

                  hasParent<SearchPage>()->candidates.setShow(false);
                  hasParent<SearchPage>()->autocompletes.setShow(true);
              }
              else{
                  CMQueryChatPeerList cmQPC;
                  std::memset(&cmQPC, 0, sizeof(cmQPC));

                  cmQPC.input.assign(query);
                  g_client->send({CM_QUERYCHATPEERLIST, cmQPC}, [query = std::move(query), this](uint8_t headCode, const uint8_t *data, size_t size)
                  {
                      switch(headCode){
                          case SM_OK:
                            {
                                hasParent<SearchPage>()->candidates.clearChild();
                                hasParent<SearchPage>()->autocompletes.clearChild();

                                for(const auto &candidate: cerealf::deserialize<SDChatPeerList>(data, size)){
                                    hasParent<SearchPage>()->appendFriendItem(candidate);
                                    hasParent<SearchPage>()->appendAutoCompletionItem(query == std::to_string(candidate.id), candidate, [&candidate, &query]
                                    {
                                        if(const auto pos = candidate.name.find(query); pos != std::string::npos){
                                            return str_printf(R"###(<par>%s<t color="red">%s</t>%s（%llu）</par>)###", candidate.name.substr(0, pos).c_str(), query.c_str(), candidate.name.substr(pos + query.size()).c_str(), to_llu(candidate.id));
                                        }
                                        else if(std::to_string(candidate.id) == query){
                                            return str_printf(R"###(<par>%s（<t color="red">%llu</t>）</par>)###", candidate.name.c_str(), to_llu(candidate.id));
                                        }
                                        else{
                                            return str_printf(R"###(<par>%s（%llu）</par>)###", candidate.name.c_str(), to_llu(candidate.id));
                                        }
                                    }());
                                }
                                break;
                            }
                        default:
                            {
                                throw fflerror("query failed in server");
                            }
                      }
                  });
              }
          },

          this,
          false,
      }

    , hint
      {{
          .x = this->input.dx(),
          .y = this->input.dy(),

          .label = u8"输入用户ID或角色名",
          .font
          {
              .id = 1,
              .size = 14,
              .color = colorf::GREY_A255,
          },

          .parent{this},
      }}
{}

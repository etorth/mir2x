#include "clientmsg.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "friendchatboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;

FriendChatBoard::SearchInputLine::SearchInputLine(Widget::VarDir argDir,

        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          SearchInputLine::WIDTH,
          SearchInputLine::HEIGHT,

          {},

          argParent,
          argAutoDelete,
      }

    , image
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000460); },
      }

    , inputbg
      {
          DIR_UPLEFT,
          0,
          0,

          this->w(),
          this->h(),

          &image,

          3,
          3,
          image.w() - 6,
          2,

          this,
          false,
      }

    , icon
      {
          DIR_NONE,
          SearchInputLine::ICON_WIDTH / 2 + SearchInputLine::ICON_MARGIN + 3,
          SearchInputLine::HEIGHT     / 2,

          std::min<int>(SearchInputLine::ICON_WIDTH, SearchInputLine::HEIGHT - 3 * 2),
          std::min<int>(SearchInputLine::ICON_WIDTH, SearchInputLine::HEIGHT - 3 * 2),

          [](const ImageBoard *) { return g_progUseDB->retrieve(0X00001200); },

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(0XFF),

          this,
          false,
      }

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
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          [this]()
          {
              dynamic_cast<SearchPage *>(parent())->candidates.setShow(true);
              dynamic_cast<SearchPage *>(parent())->autocompletes.setShow(false);
          },

          [this](std::string query)
          {
              hint.setShow(query.empty());

              if(query.empty()){
                  dynamic_cast<SearchPage *>(parent())->candidates.clearChild();
                  dynamic_cast<SearchPage *>(parent())->autocompletes.clearChild();

                  dynamic_cast<SearchPage *>(parent())->candidates.setShow(false);
                  dynamic_cast<SearchPage *>(parent())->autocompletes.setShow(true);
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
                                dynamic_cast<SearchPage *>(parent())->candidates.clearChild();
                                dynamic_cast<SearchPage *>(parent())->autocompletes.clearChild();

                                for(const auto &candidate: cerealf::deserialize<SDChatPeerList>(data, size)){
                                    dynamic_cast<SearchPage *>(parent())->appendFriendItem(candidate);
                                    dynamic_cast<SearchPage *>(parent())->appendAutoCompletionItem(query == std::to_string(candidate.id), candidate, [&candidate, &query]
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
      {
          this->input.dir(),
          this->input.dx(),
          this->input.dy(),

          u8"输入用户ID或角色名",
          1,
          14,
          0,

          colorf::GREY + colorf::A_SHF(255),

          this,
          false,
      }
{}

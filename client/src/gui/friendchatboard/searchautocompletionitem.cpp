#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "searchpage.hpp"
#include "searchautocompletionitem.hpp"
#include "friendchatboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

SearchAutoCompletionItem::SearchAutoCompletionItem(Widget::VarDir argDir,

        Widget::VarInt argX,
        Widget::VarInt argY,

        bool argByID,
        SDChatPeer argCandidate,

        const char *argLabelXMLStr,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .w = SearchAutoCompletionItem::WIDTH,
          .h = SearchAutoCompletionItem::HEIGHT,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , byID(argByID)
    , candidate(std::move(argCandidate))

    , background
      {{
          .w = this->w(),
          .h = this->h(),

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
          {
              if(Widget::ROIMap{.x=drawDstX, .y=drawDstY, .ro{roi()}}.in(SDLDeviceHelper::getMousePLoc())){
                  g_sdlDevice->fillRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
              }
              else{
                  g_sdlDevice->fillRectangle(colorf::GREY               + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(32), drawDstX, drawDstY, w(), h());
              }
          },

          .parent{this},
      }}

    , icon
      {{
          .dir = DIR_NONE,
          .x = SearchAutoCompletionItem::ICON_WIDTH / 2 + SearchAutoCompletionItem::ICON_MARGIN + 3,
          .y = SearchAutoCompletionItem::HEIGHT     / 2,

          .w = std::min<int>(SearchAutoCompletionItem::ICON_WIDTH, SearchAutoCompletionItem::HEIGHT - 3 * 2),
          .h = std::min<int>(SearchAutoCompletionItem::ICON_WIDTH, SearchAutoCompletionItem::HEIGHT - 3 * 2),

          .texLoadFunc = [](const Widget *) { return g_progUseDB->retrieve(0X00001200); },

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , label
      {{
          .x = 3 + SearchAutoCompletionItem::ICON_MARGIN + SearchAutoCompletionItem::ICON_WIDTH + SearchAutoCompletionItem::GAP,
          .y = 3,

          .font
          {
              .id = 1,
              .size = 14,
          },

          .parent{this},
      }}
{
    if(str_haschar(argLabelXMLStr)){
        label.loadXML(argLabelXMLStr);
    }
    else{
        label.loadXML(str_printf(R"###(<par>%s（%llu）</par>)###", candidate.name.c_str(), to_llu(candidate.id)).c_str());
    }
}

bool SearchAutoCompletionItem::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
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
                    hasParent<SearchPage>()->candidates.setShow(true);
                    hasParent<SearchPage>()->autocompletes.setShow(false);
                    hasParent<SearchPage>()->input.input.setInput(byID ? std::to_string(candidate.id).c_str() : candidate.name.c_str());
                    return consumeFocus(true);
                }
                return false;
            }
        default:
            {
                return Widget::processEventDefault(event, valid, m);
            }
    }
}

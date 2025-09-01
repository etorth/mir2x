#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "searchpage.hpp"
#include "searchautocompletionitem.hpp"
#include "friendchatboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

SearchAutoCompletionItem::SearchAutoCompletionItem(Widget::VarDir argDir,

        Widget::VarOff argX,
        Widget::VarOff argY,

        bool argByID,
        SDChatPeer argCandidate,

        const char *argLabelXMLStr,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          SearchAutoCompletionItem::WIDTH,
          SearchAutoCompletionItem::HEIGHT,

          {},

          argParent,
          argAutoDelete,
      }

    , byID(argByID)
    , candidate(std::move(argCandidate))

    , background
      {
          DIR_UPLEFT,
          0,
          0,

          this->w(),
          this->h(),

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              if(const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc(); in(mousePX, mousePY, drawDstX, drawDstY, roi())){
                  g_sdlDevice->fillRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
              }
              else{
                  g_sdlDevice->fillRectangle(colorf::GREY               + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(32), drawDstX, drawDstY, w(), h());
              }
          },

          this,
          false,
      }

    , icon
      {
          DIR_NONE,
          SearchAutoCompletionItem::ICON_WIDTH / 2 + SearchAutoCompletionItem::ICON_MARGIN + 3,
          SearchAutoCompletionItem::HEIGHT     / 2,

          std::min<int>(SearchAutoCompletionItem::ICON_WIDTH, SearchAutoCompletionItem::HEIGHT - 3 * 2),
          std::min<int>(SearchAutoCompletionItem::ICON_WIDTH, SearchAutoCompletionItem::HEIGHT - 3 * 2),

          [](const Widget *) { return g_progUseDB->retrieve(0X00001200); },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_NONE,

          this,
          false,
      }

    , label
      {
          DIR_UPLEFT,
          3 + SearchAutoCompletionItem::ICON_MARGIN + SearchAutoCompletionItem::ICON_WIDTH + SearchAutoCompletionItem::GAP,
          3,

          u8"",

          1,
          14,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }
{
    if(str_haschar(argLabelXMLStr)){
        label.loadXML(argLabelXMLStr);
    }
    else{
        label.loadXML(str_printf(R"###(<par>%s（%llu）</par>)###", candidate.name.c_str(), to_llu(candidate.id)).c_str());
    }
}

bool SearchAutoCompletionItem::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y, startDstX, startDstY, roiOpt.value())){
                    hasParent<SearchPage>()->candidates.setShow(true);
                    hasParent<SearchPage>()->autocompletes.setShow(false);
                    hasParent<SearchPage>()->input.input.setInput(byID ? std::to_string(candidate.id).c_str() : candidate.name.c_str());
                    return consumeFocus(true);
                }
                return false;
            }
        default:
            {
                return Widget::processEventDefault(event, valid, startDstX, startDstY, roiOpt.value());
            }
    }
}

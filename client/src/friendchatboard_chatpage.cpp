#include "sdldevice.hpp"
#include "friendchatboard.hpp"

extern SDLDevice *g_sdlDevice;

FriendChatBoard::ChatPage::ChatPage(dir8_t argDir,

        int argX,
        int argY,

        Widget *argParent,
        bool argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          UIPage_WIDTH,
          UIPage_HEIGHT,

          {},

          argParent,
          argAutoDelete,
      }

    , background
      {
          DIR_UPLEFT,
          0,
          0,
          UIPage_WIDTH,
          UIPage_HEIGHT,

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              g_sdlDevice->drawLine(
                      colorf::RGBA(231, 231, 189, 64),

                      drawDstX,
                      drawDstY + UIPage_HEIGHT - UIPage_MARGIN * 2 - INPUT_MARGIN * 2 - input.h() - 1,

                      drawDstX + UIPage_WIDTH,
                      drawDstY + UIPage_HEIGHT - UIPage_MARGIN * 2 - INPUT_MARGIN * 2 - input.h() - 1);

              g_sdlDevice->fillRectangle(
                      colorf::RGBA(231, 231, 189, 32),

                      drawDstX,
                      drawDstY + UIPage_HEIGHT - UIPage_MARGIN * 2 - INPUT_MARGIN * 2 - input.h(),

                      UIPage_WIDTH,
                      UIPage_MARGIN * 2 + ChatPage::INPUT_MARGIN * 2 + input.h());

              g_sdlDevice->fillRectangle(
                      colorf::BLACK + colorf::A_SHF(255),

                      drawDstX + UIPage_MARGIN,
                      drawDstY + UIPage_HEIGHT - UIPage_MARGIN - INPUT_MARGIN * 2 - input.h(),

                      UIPage_WIDTH - UIPage_MARGIN * 2,
                      ChatPage::INPUT_MARGIN * 2 + input.h(),

                      ChatPage::INPUT_CORNER);

              g_sdlDevice->drawRectangle(
                      colorf::RGBA(231, 231, 189, 96),

                      drawDstX + UIPage_MARGIN,
                      drawDstY + UIPage_HEIGHT - UIPage_MARGIN - INPUT_MARGIN * 2 - input.h(),

                      UIPage_WIDTH - UIPage_MARGIN * 2,
                      ChatPage::INPUT_MARGIN * 2 + input.h(),

                      ChatPage::INPUT_CORNER);
          },

          this,
          false,
      }

    , input
      {
          DIR_DOWNLEFT,
          UIPage_MARGIN                 + ChatPage::INPUT_MARGIN,
          UIPage_HEIGHT - UIPage_MARGIN - ChatPage::INPUT_MARGIN - 1,

          this,
          false,
      }

    , chat
      {
          DIR_UPLEFT,
          UIPage_MARGIN,
          UIPage_MARGIN,

          [this](const Widget *)
          {
              return UIPage_HEIGHT - UIPage_MARGIN * 4 - ChatPage::INPUT_MARGIN * 2 - input.h() - 1;
          },

          this,
          false,
      }
{}

bool FriendChatBoard::ChatPage::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            if(input.focus()){
                                return Widget::processEvent(event, valid);
                            }
                            else{
                                setFocus(false);
                                return input.consumeFocus(true, std::addressof(input.layout));
                            }
                        }
                    default:
                        {
                            return Widget::processEvent(event, valid);
                        }
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(input.in(event.button.x, event.button.y)){
                    setFocus(false);
                    return input.consumeFocus(true, std::addressof(input.layout));
                }

                if(in(event.button.x, event.button.y)){
                    return consumeFocus(true);
                }

                return false;
            }
        default:
            {
                return Widget::processEvent(event, valid);
            }
    }
}

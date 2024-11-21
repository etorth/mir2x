#include "sdldevice.hpp"
#include "chatpage.hpp"
#include "friendchatboard.hpp"

extern SDLDevice *g_sdlDevice;

ChatPage::ChatPage(
        Widget::VarDir  argDir,
        Widget::VarOff  argX,
        Widget::VarOff  argY,
        Widget::VarSize argW,
        Widget::VarSize argH,

        Widget *argParent,
        bool argAutoDelete)

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

    , background
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              // ChatPage = top + sepLine + bottom
              const int bottomHeight = UIPage_MARGIN + ChatPage::SEP_MARGIN + ChatPage::INPUT_MARGIN * 2 + input.h() + (showref() ? (chatref->h() + ChatPage::CHATREF_GAP) : 0);
              const int sepLineDY    = h() - bottomHeight - 1;

              g_sdlDevice->drawLine(
                      colorf::RGBA(231, 231, 189, 64),

                      drawDstX,
                      drawDstY + sepLineDY,

                      drawDstX + w(),
                      drawDstY + sepLineDY);

              g_sdlDevice->fillRectangle(
                      colorf::RGBA(231, 231, 189, 32),

                      drawDstX,
                      drawDstY + sepLineDY + 1,

                      w(),
                      bottomHeight);

              g_sdlDevice->fillRectangle(
                      colorf::BLACK + colorf::A_SHF(255),

                      drawDstX + UIPage_MARGIN,
                      drawDstY + sepLineDY + ChatPage::SEP_MARGIN,

                      w() - UIPage_MARGIN * 2,
                      ChatPage::INPUT_MARGIN * 2 + input.h(),

                      ChatPage::INPUT_CORNER);

              g_sdlDevice->drawRectangle(
                      colorf::RGBA(231, 231, 189, 96),

                      drawDstX + UIPage_MARGIN,
                      drawDstY + sepLineDY + ChatPage::SEP_MARGIN,

                      w() - UIPage_MARGIN * 2,
                      ChatPage::INPUT_MARGIN * 2 + input.h(),

                      ChatPage::INPUT_CORNER);
          },

          this,
          false,
      }

    , input
      {
          DIR_DOWNLEFT,
          UIPage_MARGIN + ChatPage::INPUT_MARGIN,
          [this](const Widget *)
          {
              return h() - UIPage_MARGIN - (showref() ? (chatref->h() + ChatPage::CHATREF_GAP) : 0) - ChatPage::INPUT_MARGIN - 1;
          },

          [this](const Widget *)
          {
              return w() - UIPage_MARGIN * 2 - ChatPage::INPUT_MARGIN * 2;
          },

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
              return w() - UIPage_MARGIN * 2;
          },

          [this](const Widget *)
          {
              return h() - UIPage_MARGIN * 2 - ChatPage::SEP_MARGIN * 2 - 1 - ChatPage::INPUT_MARGIN * 2 - input.h() - (showref() ? (chatref->h() + ChatPage::CHATREF_GAP) : 0);
          },

          this,
          false,
      }
{}

bool ChatPage::showref() const
{
    return chatref && chatref->show();
}

bool ChatPage::showmenu() const
{
    return menu && menu->show();
}

std::optional<uint64_t> ChatPage::refopt() const
{
    if(showref()){
        return chatref->refer();
    }
    return std::nullopt;
}

void ChatPage::enableChatRef(uint64_t refMsgID, std::string xmlStr)
{
    if(chatref){
        removeChild(chatref, true);
    }
    chatref = ChatPage::createChatItemRef(refMsgID, std::move(xmlStr), this, true);
}

void ChatPage::disableChatRef()
{
    if(chatref){
        removeChild(chatref, true);
        chatref = nullptr;
    }
}

void ChatPage::afterResizeDefault()
{
    chat .afterResize();
    input.afterResize();

    if(!showref()){
        return;
    }

    enableChatRef(chatref->refer(), chatref->getXML());
}

bool ChatPage::processEventDefault(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(showref()){
        if(chatref->processEvent(event, valid)){
            return true;
        }
    }

    if(showmenu()){
        if(menu->processEvent(event, valid)){
            return true;
        }
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            if(input.focus()){
                                return Widget::processEventDefault(event, valid);
                            }
                            else{
                                setFocus(false);
                                return input.consumeFocus(true, std::addressof(input.layout));
                            }
                        }
                    default:
                        {
                            return Widget::processEventDefault(event, valid);
                        }
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(input.in(event.button.x, event.button.y)){
                    setFocus(false);
                    return input.consumeFocus(true, std::addressof(input.layout));
                }

                if(chat.processEvent(event, true)){
                    return true;
                }

                if(in(event.button.x, event.button.y)){
                    if(menu){
                        removeChild(menu, true);
                        menu = nullptr;
                    }
                    return consumeFocus(true);
                }

                return false;
            }
        default:
            {
                return Widget::processEventDefault(event, valid);
            }
    }
}

ChatItemRef *ChatPage::createChatItemRef(uint64_t msgID, std::string xmlStr, Widget *self, bool autoDelete)
{
    return new ChatItemRef
    {
        DIR_DOWNLEFT,
        UIPage_MARGIN,
        [self](const Widget *){ return self->h() - UIPage_MARGIN - 1; },

        self->w() - 24, // can not stretch
        true,
        true,

        msgID,
        xmlStr.c_str(),

        self,
        autoDelete,
    };
}

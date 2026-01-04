#include "sdldevice.hpp"
#include "chatpage.hpp"
#include "friendchatboard.hpp"

extern SDLDevice *g_sdlDevice;

ChatPage::ChatPage(
        Widget::VarDir  argDir,
        Widget::VarInt  argX,
        Widget::VarInt  argY,
        Widget::VarSizeOpt argW,
        Widget::VarSizeOpt argH,

        Widget *argParent,
        bool argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = std::move(argW),
          .h = std::move(argH),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , background
      {{
          .w = [this](const Widget *){ return w(); },
          .h = [this](const Widget *){ return h(); },

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
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

          .parent{this},
      }}

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
        removeChild(chatref->id(), true);
    }
    chatref = ChatPage::createChatItemRef(refMsgID, std::move(xmlStr), this, true);
}

void ChatPage::disableChatRef()
{
    if(chatref){
        removeChild(chatref->id(), true);
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

bool ChatPage::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(showref()){
        if(chatref->processEventParent(event, valid, m)){
            return true;
        }
    }

    if(showmenu()){
        if(menu->processEvent(event, valid, m)){
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
                                return Widget::processEventDefault(event, valid, m);
                            }
                            else{
                                setFocus(false);
                                return input.consumeFocus(true, std::addressof(input.layout));
                            }
                        }
                    default:
                        {
                            return Widget::processEventDefault(event, valid, m);
                        }
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m.create(input.roi()).in(event.button.x, event.button.y)){
                    setFocus(false);
                    return input.consumeFocus(true, std::addressof(input.layout));
                }

                if(chat.processEventParent(event, true, m)){
                    return true;
                }

                if(m.in(event.button.x, event.button.y)){
                    if(menu){
                        removeChild(menu->id(), true);
                        menu = nullptr;
                    }
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

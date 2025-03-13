#include "sdldevice.hpp"
#include "frienditem.hpp"
#include "friendchatboard.hpp"

extern SDLDevice *g_sdlDevice;

FriendItem::FriendItem(
        Widget::VarDir  argDir,
        Widget::VarOff  argX,
        Widget::VarOff  argY,
        Widget::VarSize argW,

        const SDChatPeerID &argCPID,

        const char8_t *argNameStr,
        std::function<SDL_Texture *(const ImageBoard *)> argLoadImageFunc,

        std::function<void(FriendItem *)> argOnClick,
        std::pair<Widget *, bool> argFuncWidget,

        Widget *argParent,
        bool argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          std::move(argW),
          FriendItem::HEIGHT,

          {
              {
                  argFuncWidget.first,
                  DIR_RIGHT,
                  UIPage_MIN_WIDTH - UIPage_MARGIN * 2 - FriendItem::FUNC_MARGIN - 1,
                  FriendItem::HEIGHT / 2,
                  argFuncWidget.second,
              },
          },

          argParent,
          argAutoDelete,
      }

    , cpid(argCPID)
    , funcWidgetID(argFuncWidget.first ? argFuncWidget.first->id() : 0)
    , onClick(std::move(argOnClick))

    , hovered
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              if(const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc(); in(mousePX, mousePY)){
                  g_sdlDevice->fillRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
              }
              else{
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(32), drawDstX, drawDstY, w(), h());
              }
          },

          this,
          false,
      }

    , avatar
      {
          DIR_UPLEFT,
          FriendItem::ITEM_MARGIN,
          FriendItem::ITEM_MARGIN,

          FriendItem::AVATAR_WIDTH,
          FriendItem::HEIGHT - FriendItem::ITEM_MARGIN * 2,

          std::move(argLoadImageFunc),

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(0XFF),

          this,
          false,
      }

    , name
      {
          DIR_LEFT,
          FriendItem::ITEM_MARGIN + FriendItem::AVATAR_WIDTH + FriendItem::GAP,
          FriendItem::HEIGHT / 2,

          argNameStr,

          1,
          14,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }
{}

void FriendItem::setFuncWidget(Widget *argFuncWidget, bool argAutoDelete)
{
    clearChild([this](const Widget *widget, bool)
    {
        return this->funcWidgetID == widget->id();
    });

    addChild(argFuncWidget, argAutoDelete);
}

bool FriendItem::processEventDefault(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(Widget::processEventDefault(event, valid)){
                    return consumeFocus(true);
                }
                else if(in(event.button.x, event.button.y)){
                    if(onClick){
                        onClick(this);
                    }
                    return consumeFocus(true);
                }
                else{
                    return false;
                }
            }
        default:
            {
                return Widget::processEventDefault(event, valid);
            }
    }
}

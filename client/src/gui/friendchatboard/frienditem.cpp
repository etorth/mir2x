#include "sdldevice.hpp"
#include "frienditem.hpp"
#include "friendchatboard.hpp"

extern SDLDevice *g_sdlDevice;

FriendItem::FriendItem(
        Widget::VarDir  argDir,
        Widget::VarInt  argX,
        Widget::VarInt  argY,
        Widget::VarSizeOpt argW,

        const SDChatPeerID &argCPID,

        const char8_t *argNameStr,
        std::function<SDL_Texture *(const Widget *)> argLoadImageFunc,

        std::function<void(FriendItem *)> argOnClick,
        std::pair<Widget *, bool> argFuncWidget,

        Widget *argParent,
        bool argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = std::move(argW),
          .h = FriendItem::HEIGHT,

          .childList
          {
              {
                  argFuncWidget.first,
                  DIR_RIGHT,
                  UIPage_MIN_WIDTH - UIPage_MARGIN * 2 - FriendItem::FUNC_MARGIN - 1,
                  FriendItem::HEIGHT / 2,
                  argFuncWidget.second,
              },
          },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , cpid(argCPID)
    , funcWidgetID(argFuncWidget.first ? argFuncWidget.first->id() : 0)
    , onClick(std::move(argOnClick))

    , hovered
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(Widget::ROIMap{.x=drawDstX, .y=drawDstY, .ro{self->roi()}}.in(SDLDeviceHelper::getMousePLoc())){
                  g_sdlDevice->fillRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
              }
              else{
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(32), drawDstX, drawDstY, w(), h());
              }
          },

          .parent{this},
      }}

    , avatar
      {{
          .x = FriendItem::ITEM_MARGIN,
          .y = FriendItem::ITEM_MARGIN,

          .w = FriendItem::AVATAR_WIDTH,
          .h = FriendItem::HEIGHT - FriendItem::ITEM_MARGIN * 2,

          .texLoadFunc = std::move(argLoadImageFunc),

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , name
      {{
          .dir = DIR_LEFT,
          .x = FriendItem::ITEM_MARGIN + FriendItem::AVATAR_WIDTH + FriendItem::GAP,
          .y = FriendItem::HEIGHT / 2,

          .label = argNameStr,
          .font
          {
              .id = 1,
              .size = 14,
              .color = colorf::WHITE_A255,
          },

          .parent{this},
      }}
{}

void FriendItem::setFuncWidget(Widget *argFuncWidget, bool argAutoDelete)
{
    clearChild([this](const Widget *widget, bool)
    {
        return this->funcWidgetID == widget->id();
    });

    addChild(argFuncWidget, argAutoDelete);
}

bool FriendItem::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
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
                if(Widget::processEventDefault(event, valid, m)){
                    return consumeFocus(true);
                }
                else if(m.in(event.button.x, event.button.y)){
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
                return Widget::processEventDefault(event, valid, m);
            }
    }
}

#include "sdldevice.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern SDLDevice *g_sdlDevice;

FriendChatBoard::ChatItemRef::ChatItemRef(dir8_t argDir,
        int argX,
        int argY,
        int argW,

        std::string argLayoutXML,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,
          argW,
          0, // setup later

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

          [](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->fillRectangle(colorf::GREY + colorf::A_SHF(128), dstDrawX, dstDrawY, self->w(), self->h(), ChatItemRef::CORNER);
          },

          this,
          false,
      }

    , cross
      {
          DIR_UPLEFT, // ignored
          0,
          0,

          u8"x",

          1,
          12,
          0,

          colorf::WHITE + colorf::A_SHF(255),
      }

    , crossBg
      {
          DIR_UPLEFT, // ignored
          0,
          0,

          ChatItemRef::BUTTON_R * 2 + 1,
          ChatItemRef::BUTTON_R * 2 + 1,

          [](const Widget *, int drawDstX, int drawDstY)
          {
              if(auto coverTexPtr = g_sdlDevice->getCover(ChatItemRef::BUTTON_R, 360)){
                  g_sdlDevice->drawTexture(coverTexPtr, drawDstX, drawDstY);
              }
          },
      }

    , crossButtonGfx
      {
          DIR_UPLEFT,
          0,
          0,

          ChatItemRef::BUTTON_R * 2 + 1,
          ChatItemRef::BUTTON_R * 2 + 1,

          {
              {&crossBg, DIR_NONE, ChatItemRef::BUTTON_R, ChatItemRef::BUTTON_R, false},
              {&cross  , DIR_NONE, ChatItemRef::BUTTON_R, ChatItemRef::BUTTON_R, false},
          },
      }

    , crossButton
      {
          DIR_RIGHT,
          [this](const Widget *){ return w() - ChatItemRef::MARGIN - 1; },
          [this](const Widget *){ return h() / 2;                       },

          {
              &crossButtonGfx,
              &crossButtonGfx,
              &crossButtonGfx,
          },

          {
              std::nullopt,
              std::nullopt,
              0X01020000 + 105,
          },

          [this](Widget *)
          {
              cross.setFont(10);
          },

          [this](Widget *)
          {
              cross.setFont(12);
          },

          [this](Widget *)
          {
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          false,

          this,
          false,
      }

    , message
      {
          DIR_UPLEFT,
          ChatItemRef::MARGIN,
          ChatItemRef::MARGIN,

          std::max<int>(1, argW - ChatItemRef::MARGIN - ChatItemRef::BUTTON_MARGIN * 2 - ChatItemRef::BUTTON_R * 2 - 1),
          argLayoutXML.c_str(),
          0,

          {},

          false,
          false,
          false,
          false,

          0,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_LEFT,
          0,
          0,

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          nullptr,
          nullptr,

          this,
          false,
      }
{
    setH([this](const Widget *)
    {
        return ChatItemRef::MARGIN * 2 + message.h();
    });
}

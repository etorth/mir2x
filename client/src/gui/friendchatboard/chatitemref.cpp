#include "sdldevice.hpp"
#include "processrun.hpp"
#include "chatitemref.hpp"

extern SDLDevice *g_sdlDevice;

ChatItemRef::ChatItemRef(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        int argMaxWidth,

        bool argForceWidth,
        bool argShowButton,

        std::string argLayoutXML,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          0, // setup later
          0, //

          {},

          argParent,
          argAutoDelete,
      }

    , m_crossBgColor(colorf::GREY + colorf::A_SHF(255))
    , m_background
      {
          DIR_UPLEFT,
          0,
          0,
          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->fillRectangle(colorf::GREY + colorf::A_SHF(200), dstDrawX, dstDrawY, self->w(), self->h(), ChatItemRef::CORNER);
          },

          this,
          false,
      }

    , m_cross
      {
          DIR_UPLEFT, // ignored
          0,
          0,

          u8"×", // multiplication sign for better symmetry

          1,
          ChatItemRef::CROSS_FONT_SIZES[0],
          0,

          colorf::WHITE + colorf::A_SHF(255),
      }

    , m_crossBg
      {
          DIR_UPLEFT, // ignored
          0,
          0,

          ChatItemRef::BUTTON_D,
          ChatItemRef::BUTTON_D,

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              if(auto texPtr = g_sdlDevice->getCover(ChatItemRef::BUTTON_R, 360)){
                  const SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND);
                  const SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, m_crossBgColor);
                  g_sdlDevice->drawTexture(texPtr, drawDstX, drawDstY);
              }
          },
      }

    , m_crossButtonGfx
      {
          DIR_UPLEFT,
          0,
          0,

          ChatItemRef::BUTTON_D,
          ChatItemRef::BUTTON_D,

          {
              {&m_crossBg, DIR_NONE, [this](const Widget *){ return m_crossButtonGfx.w() / 2; }, [this](const Widget *){ return m_crossButtonGfx.h() / 2; }, false},
              {&m_cross  , DIR_NONE, [this](const Widget *){ return m_crossButtonGfx.w() / 2; }, [this](const Widget *){ return m_crossButtonGfx.h() / 2; }, false},
          },
      }

    , m_crossButton
      {
          DIR_RIGHT,
          [this](const Widget *){ return w() - ChatItemRef::BUTTON_MARGIN - 1; },
          [this](const Widget *){ return h() / 2;                              },

          {
              &m_crossButtonGfx,
              &m_crossButtonGfx,
              &m_crossButtonGfx,
          },

          {
              std::nullopt,
              std::nullopt,
              0X01020000 + 105,
          },

          [this](Widget *)
          {
              m_crossBgColor = colorf::BLUE + colorf::A_SHF(128);
              m_cross.setFontSize(ChatItemRef::CROSS_FONT_SIZES[1]);
          },

          [this](Widget *)
          {
              m_crossBgColor = colorf::GREY + colorf::A_SHF(255);
              m_cross.setFontSize(ChatItemRef::CROSS_FONT_SIZES[0]);
          },

          [this](Widget *, bool clickDone)
          {
              if(clickDone){
              }
              else{
                  m_cross.setFontSize(ChatItemRef::CROSS_FONT_SIZES[2]);
              }
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

    , m_message
      {
          DIR_UPLEFT,
          ChatItemRef::MARGIN,
          ChatItemRef::MARGIN,

          std::max<int>(1, argShowButton ? (argMaxWidth - ChatItemRef::MARGIN - ChatItemRef::BUTTON_MARGIN * 2 - ChatItemRef::BUTTON_D)
                                         : (argMaxWidth - ChatItemRef::MARGIN * 2)),
          argLayoutXML.c_str(),
          0,

          {},

          false,
          false,
          false,
          false,

          1,
          10,
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
    m_crossButton.setShow(argShowButton);

    if(argForceWidth){
        setW(std::max<int>(0, argMaxWidth));
    }
    else{
        setW([argShowButton, this](const Widget *)
        {
            if(argShowButton){
                return ChatItemRef::MARGIN + m_message.w() + ChatItemRef::BUTTON_MARGIN * 2 + ChatItemRef::BUTTON_D;
            }
            else{
                return ChatItemRef::MARGIN * 2 + m_message.w();
            }
        });
    }

    setH([this](const Widget *)
    {
        return std::max<int>(ChatItemRef::MARGIN * 2 + m_message.h(), ChatItemRef::BUTTON_D);
    });
}

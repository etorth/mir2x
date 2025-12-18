#include "sdldevice.hpp"
#include "processrun.hpp"
#include "chatitemref.hpp"

extern SDLDevice *g_sdlDevice;

ChatItemRef::ChatItemRef(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        int argMaxWidth,

        bool argForceWidth,
        bool argShowButton,

        uint64_t argRef,
        std::string argLayoutXML,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_crossBgColor(colorf::GREY + colorf::A_SHF(255))
    , m_background
      {{
          .w = [this](const Widget *){ return w(); },
          .h = [this](const Widget *){ return h(); },

          .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->fillRectangle(colorf::GREY + colorf::A_SHF(200), dstDrawX, dstDrawY, self->w(), self->h(), ChatItemRef::CORNER);
          },

          .parent{this},
      }}

    , m_cross
      {{
          .label = u8"×", // multiplication sign for better symmetry
          .font
          {
              .id = 1,
              .size = ChatItemRef::CROSS_FONT_SIZES[0],
          },
      }}

    , m_crossBg
      {{
          .w = ChatItemRef::BUTTON_D,
          .h = ChatItemRef::BUTTON_D,

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
          {
              if(auto texPtr = g_sdlDevice->getCover(ChatItemRef::BUTTON_R, 360)){
                  const SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND);
                  const SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, m_crossBgColor);
                  g_sdlDevice->drawTexture(texPtr, drawDstX, drawDstY);
              }
          },
      }}

    , m_crossButtonGfx
      {{
          .w = [this](const Widget *){ return m_crossBg.w(); },
          .h = [this](const Widget *){ return m_crossBg.h(); },

          .childList
          {
              {&m_crossBg, DIR_NONE, [this](const Widget *){ return m_crossButtonGfx.w() / 2; }, [this](const Widget *){ return m_crossButtonGfx.h() / 2; }, false},
              {&m_cross  , DIR_NONE, [this](const Widget *){ return m_crossButtonGfx.w() / 2; }, [this](const Widget *){ return m_crossButtonGfx.h() / 2; }, false},
          },
      }}

    , m_crossButton
      {{
          .dir = DIR_RIGHT,
          .x = [this]{ return w() - ChatItemRef::BUTTON_MARGIN - 1; },
          .y = [this]{ return h() / 2;                              },

          .gfxList
          {
              &m_crossButtonGfx,
              &m_crossButtonGfx,
              &m_crossButtonGfx,
          },

          .onOverIn = [this](Widget *)
          {
              m_crossBgColor = colorf::BLUE + colorf::A_SHF(64);
              m_cross.setFontSize(ChatItemRef::CROSS_FONT_SIZES[1]);
          },

          .onOverOut = [this](Widget *)
          {
              m_crossBgColor = colorf::GREY + colorf::A_SHF(255);
              m_cross.setFontSize(ChatItemRef::CROSS_FONT_SIZES[0]);
          },

          .onClick = [this](Widget *, bool clickDone, int)
          {
              if(clickDone){
              }
              else{
                  m_cross.setFontSize(ChatItemRef::CROSS_FONT_SIZES[2]);
              }
          },

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
          },

          .parent{this},
      }}

    , m_refer(argRef)

    , m_message
      {{
          .x = ChatItemRef::MARGIN,
          .y = ChatItemRef::MARGIN,

          .lineWidth = std::max<int>(1, argShowButton ? (argMaxWidth - ChatItemRef::MARGIN - ChatItemRef::BUTTON_MARGIN * 2 - ChatItemRef::BUTTON_D)
                                                      : (argMaxWidth - ChatItemRef::MARGIN * 2)),
          .initXML = argLayoutXML.c_str(),

          .font
          {
              .id = 1,
              .size = 10,
          },

          .parent{this},
      }}
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

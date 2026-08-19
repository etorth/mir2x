#include "widget.hpp"
#include "colorf.hpp"
#include "utf8f.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "gui/controlboard/controlboard.hpp"
#include "inputstringboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

InputStringBoard::InputStringBoard(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        bool argSecurity,

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

    , m_bg
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X07000000);
          },

          .blendMode = SDL_BLENDMODE_BLEND,
          .parent{this},
      }}

    , m_textInfo
      {{
          .dir = DIR_NONE,

          .x = [this](){ return w() / 2; },
          .y = 120,

          .lineWidth = 250,

          .font
          {
              .id = 1,
              .size = 12,
          },

          .lineAlign = LALIGN_JUSTIFY,
          .parent{this},
      }}

    , m_inputBg
      {{
          .x = 22,
          .y = 225,

          .w = 315,
          .h = 23,

          .drawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(32), drawDstX, drawDstY, self->w(), self->h());
          },

          .parent{this},
      }}

    , m_input
      {{
          .x = [this]{ return m_inputBg.dx(); },
          .y = [this]{ return m_inputBg.dy(); },

          .w = [this]{ return m_inputBg.w(); },
          .h = [this]{ return m_inputBg.h(); },

          .font
          {
              .id = 1,
              .size = 14,
          },

          .onCR = [this]
          {
              acceptInput();
              setShow(false);
          },

          .codeXfer = [argSecurity, codeMask = utf8f::str2code("*")](uint32_t codePoint)
          {
              return argSecurity ? codeMask : codePoint;
          },

          .parent{this},
      }}

    , m_yesButton
      {{
          .x = 66,
          .y = 190,

          .texIDList
          {
              .off  = 0X07000001,
              .on   = 0X07000002,
              .down = 0X07000003,
          },

          .onTrigger = [this](Widget *, int)
          {
              acceptInput();
              setShow(false);
          },

          .parent{this},
      }}

    , m_nopButton
      {{
          .x = 212,
          .y = 190,

          .texIDList
          {
              .off  = 0X07000004,
              .on   = 0X07000005,
              .down = 0X07000006,
          },

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
              m_input.clear();
              m_input.setFocus(false);

              m_onAccept = {};
              m_input.setValidateFunc({});
              if(auto onReject = std::move(m_onReject)){
                  onReject();
              }
          },

          .parent{this},
      }}
{
    setShow(false);
    setSize([this]{ return m_bg.w(); },
            [this]{ return m_bg.h(); });

    m_inputBg.setShow([this]
    {
        return m_input.focus();
    });
}

void InputStringBoard::acceptInput()
{
    const std::string fullInput = m_input.getRawString();
    const auto inputPos = fullInput.find_first_not_of(" \n\r\t");
    const std::string realInput = (inputPos == std::string::npos) ? "" : fullInput.substr(inputPos);

    m_input.clear();
    m_input.setFocus(false);

    m_onReject = {};
    m_input.setValidateFunc({});
    if(auto onAccept = std::move(m_onAccept)){
        onAccept(to_u8rawstr(realInput));
    }
}

void InputStringBoard::waitInput(InputStringBoard::WaitInputArgs args)
{
    m_textInfo.loadXML(to_cstr(args.layoutString), 0);
    m_onAccept = std::move(args.onAccept);
    m_onReject = std::move(args.onReject);

    m_input.setShow(true);
    m_input.setActive(true);
    m_input.setValidateFunc(std::move(args.validate));
    if(args.security){
        m_input.setCodeXferFunc([maskCode = utf8f::str2code("*")](uint32_t){ return maskCode; });
    } else {
        m_input.setCodeXferFunc(nullptr);
    }

    clear();
    setShow(true);
    setFocus(true);
}

void InputStringBoard::waitChoice(
        std::u8string layoutString,
        std::function<void()> onAccept,
        std::function<void()> onReject)
{
    m_textInfo.loadXML(to_cstr(layoutString), 0);
    m_onAccept = [onAccept = std::move(onAccept)](std::u8string)
    {
        if(onAccept){
            onAccept();
        }
    };
    m_onReject = std::move(onReject);

    clear();
    m_input.setFocus(false);
    m_input.setActive(false);
    m_input.setShow(false);
    m_input.setValidateFunc({});

    setShow(true);
    setFocus(true);
}

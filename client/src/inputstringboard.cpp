#include "widget.hpp"
#include "colorf.hpp"
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

          .security = std::move(argSecurity),
          .font
          {
              .id = 1,
              .size = 14,
          },

          .onCR = [this]
          {
              inputLineDone();
              setShow(false);
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
              inputLineDone();
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

void InputStringBoard::inputLineDone()
{
    const std::string fullInput = m_input.getPasswordString();
    const auto inputPos = fullInput.find_first_not_of(" \n\r\t");
    const std::string realInput = (inputPos == std::string::npos) ? "" : fullInput.substr(inputPos);

    m_input.clear();
    m_input.setFocus(false);

    if(m_onDone){
        m_onDone(to_u8cstr(realInput));
    }
}

void InputStringBoard::waitInput(std::u8string layoutString, std::function<void(std::u8string)> onDone)
{
    m_textInfo.loadXML(to_cstr(layoutString), 0);
    m_onDone = std::move(onDone);

    clear();
    setShow(true);
    setFocus(true);
}

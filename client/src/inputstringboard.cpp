#include "widget.hpp"
#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "controlboard.hpp"
#include "inputstringboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

InputStringBoard::InputStringBoard(dir8_t dir, int x, int y, bool security, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          dir,
          x,
          y,
          0,
          0,
          {},

          widgetPtr,
          autoDelete
      }

    , m_input
      {
          DIR_UPLEFT,
          22,
          225,
          315,
          23,
          security,

          1,
          14,

          0,
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          [this]()
          {
              inputLineDone();
              setShow(false);
          },

          this,
          false,
      }

    , m_yesButton
      {
          DIR_UPLEFT,
          66,
          190,
          {0X07000001, 0X07000002, 0X07000003},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](Widget *)
          {
              inputLineDone();
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          false,
          false,

          this,
          false,
      }

    , m_nopButton
      {
          DIR_UPLEFT,
          212,
          190,
          {0X07000004, 0X07000005, 0X07000006},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](Widget *)
          {
              setShow(false);
              m_input.clear();
          },

          0,
          0,
          0,
          0,

          true,
          false,
          false,

          this,
          false,
      }
{
    setShow(false);
    if(auto texPtr = g_progUseDB->retrieve(0X07000000)){
        std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid purchase count board frame texture");
    }
}

void InputStringBoard::update(double ms)
{
    m_input.update(ms);
}

void InputStringBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X07000000)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }

    m_input    .draw();
    m_yesButton.draw();
    m_nopButton.draw();

    const LayoutBoard textInfoBoard
    {
        DIR_UPLEFT,
        0,
        0,
        250,

        to_cstr(m_parXMLString),
        0,

        {},
        false,
        false,
        false,
        false,

        1,
        12,
        0,
        colorf::WHITE + colorf::A_SHF(255),
        0,

        LALIGN_JUSTIFY,
    };

    textInfoBoard.drawAt(DIR_NONE, x() + w() / 2, y() + 120);

    if(m_input.focus()){
        g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(32), m_input.x(), m_input.y(), m_input.w(), m_input.h());
    }
}

bool InputStringBoard::processEventDefault(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_input.processEvent(event, valid)){
        return true;
    }

    if(m_yesButton.processEvent(event, valid)){
        return true;
    }

    if(m_nopButton.processEvent(event, valid)){
        return true;
    }
    return true;
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
    m_parXMLString = std::move(layoutString);
    m_onDone = std::move(onDone);

    clear();
    setShow(true);
    setFocus(true);
}

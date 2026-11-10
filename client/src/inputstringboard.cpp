#include "widget.hpp"
#include "colorf.hpp"
#include "utf8f.hpp"
#include "progusedb.hpp"
#include "sdldevice.hpp"
#include "gui/controlboard/controlboard.hpp"
#include "inputstringboard.hpp"

extern ProgUseDB *g_progUseDB;
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
          .y = [this](){ return contentY() + textInfoH() / 2; },

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
          .x = [this]
          {
              if(m_onAccept && m_onReject){
                  return 66;
              }
              else{
                  return (w() - m_yesButton.w()) / 2;
              }
          },
          .y = 190,

          .texAniOpt = 0X07000001,

          .onTrigger = [this](Widget *, int)
          {
              acceptInput();
              setShow(false);
          },

          .parent{this},
      }}

    , m_nopButton
      {{
          .x = [this]
          {
              if(m_onAccept && m_onReject){
                  return 212;
              }
              else{
                  return (w() - m_nopButton.w()) / 2;
              }
          },
          .y = 190,

          .texAniOpt = 0X07000004,

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
              m_input.clear();
              m_input.setFocus(false);

              m_modal = false;
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

    m_yesButton.setShow([this]{ return static_cast<bool>(m_onAccept) || !m_onReject; });
    m_nopButton.setShow([this]{ return static_cast<bool>(m_onReject)               ; });

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

    m_modal = false;
    m_onReject = {};
    m_input.setValidateFunc({});
    if(auto onAccept = std::move(m_onAccept)){
        onAccept(to_u8rawstr(realInput));
    }
}

void InputStringBoard::resetChoice()
{
    if(m_choice){
        removeChild(m_choice->id(), true);
        m_choice = nullptr;
    }
}

size_t InputStringBoard::selectedChoice() const
{
    size_t index = 0;
    size_t selected = 0;

    if(m_choice){
        m_choice->foreachRadioWidget([&index, &selected, this](const Widget *widget)
        {
            if(widget == m_choice->getter()){
                selected = index;
                return true;
            }

            index++;
            return false;
        });
    }
    return selected;
}

int InputStringBoard::textInfoH() const
{
    return m_textInfo.empty() ? 0 : m_textInfo.h();
}

int InputStringBoard::choiceGap() const
{
    return (textInfoH() > 0 && m_choice) ? 10 : 0;
}

int InputStringBoard::contentY() const
{
    // center prompt and choices vertically around y = 120
    return 120 - (textInfoH() + choiceGap() + (m_choice ? m_choice->h() : 0)) / 2;
}

void InputStringBoard::waitInput(InputStringBoard::WaitInputArgs args)
{
    resetChoice();
    m_textInfo.loadXML(to_cstr(args.layoutString), 0);
    m_onAccept = std::move(args.onAccept);
    m_onReject = std::move(args.onReject);
    m_modal = args.modal;

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

void InputStringBoard::waitChoice(std::u8string prompt, std::vector<std::u8string> choiceList, bool modal, std::function<void(size_t)> onAccept, std::function<void()> onReject)
{
    fflassert(!prompt.empty() || !choiceList.empty());

    if(prompt.empty()){
        m_textInfo.clear();
    }
    else{
        m_textInfo.loadXML(to_cstr(prompt), 0);
    }

    resetChoice();
    if(!choiceList.empty()){
        m_choice = new RadioSelector
        {
            DIR_UPLEFT,
            [this]{ return (w() - m_choice->w()) / 2; },
            [this]{ return contentY() + textInfoH() + choiceGap(); },

            5,
            5,

            {},

            nullptr,
            nullptr,
            nullptr,

            this,
            true,
        };

        for(const auto &choice: choiceList){
            auto choiceBoard = new LayoutBoard
            {{
                .initXML = to_cstr(choice),
                .font
                {
                    .id = 1,
                    .size = 12,
                },
            }};

            if(!m_choice->getter()){
                m_choice->setter(choiceBoard);
            }
            m_choice->append(choiceBoard, true);
        }
    }

    if(onAccept){
        m_onAccept = [onAccept = std::move(onAccept), this](std::u8string){ onAccept(selectedChoice()); };
    }
    else{
        m_onAccept = nullptr;
    }
    m_onReject = std::move(onReject);
    m_modal = modal;

    clear();
    m_input.setFocus(false);
    m_input.setActive(false);
    m_input.setShow(false);
    m_input.setValidateFunc({});

    setShow(true);
    setFocus(true);
}

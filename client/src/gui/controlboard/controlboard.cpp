#include "log.hpp"
#include "controlboard.hpp"
#include "processrun.hpp"

extern Log *g_log;
extern SDLDevice *g_sdlDevice;

ControlBoard::ControlBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .dir = DIR_DOWNLEFT,

          .y = [this]{ return g_sdlDevice->getRendererHeight() - 1; },
          .w = [this]{ return g_sdlDevice->getRendererWidth ()    ; },
          .h = [this]
          {
              if(m_minimize){
                  return CBTitle::UP_HEIGHT + 10;
              }

              else if(m_expand){
                  return m_middleExpand.h() + CBTitle::UP_HEIGHT;
              }

              else{
                  return m_middle.h() + CBTitle::UP_HEIGHT;
              }
          },

          .attrs
          {
              .inst
              {
                  .moveOnFocus = false,
                  .update = [this](Widget *, double ms)
                  {
                      m_logBoard.update(ms);
                      m_cmdBoard.update(ms);
                      Widget::updateDefault(ms);
                  },
              },
          },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_logBoard
      {{
          .lineAlign = LALIGN_JUSTIFY,
      }}

    , m_cmdBoard
      {{
          .canEdit = true,
          .enableIME = [this]
          {
              return m_processRun->getRuntimeConfig<RTCFG_PINYIN>();
          },

          .lineAlign = LALIGN_JUSTIFY,

          .onCR = [this]
          {
              onInputDone();
          },

          .onCursorMove = [this]
          {
              if(m_middle.show()){
                  m_middle.onCmdCursorMove();
              }
              else if(m_middleExpand.show()){
                    m_middleExpand.onCmdCursorMove();
              }
          },
      }}

    , m_left
      {
          DIR_DOWNLEFT,
          0,
          [this]{ return h() - 1; },

          argProc,
          this,
          false,
      }

    , m_right
      {
          DIR_DOWNRIGHT,
          [this]{ return w() - 1; },
          [this]{ return h() - 1; },

          argProc,
          this,
          false,
      }

    , m_middle
      {
          DIR_DOWNLEFT,
          [this]
          {
              return m_left.w();
          },

          [this]
          {
              return h() - 1;
          },

          [this]
          {
              return w() - m_left.w() - m_right.w();
          },

          argProc,
          this,
          false,
      }

    , m_middleExpand
      {
          DIR_DOWNLEFT,
          [this]
          {
              return m_left.w();
          },

          [this]
          {
              return h() - 1;
          },

          [this]
          {
              return w() - m_left.w() - m_right.w();
          },

          argProc,
          this,
          false,
      }

    , m_title
      {
          DIR_UP,
          [this]
          {
              const auto  fullW =         w();
              const auto  leftW = m_left .w();
              const auto rightW = m_right.w();

              return leftW + (fullW - leftW - rightW) / 2;
          },

          0,
          argProc,

          this,
          false,
      }
{
    m_logBoard.setLineWidth(m_middle.getLogWindowWidth());
}

void ControlBoard::addXMLLog(const char *log)
{
    fflassert(str_haschar(log));
    m_logBoard.addLayoutXML(m_logBoard.parCount(), {0, 0, 0, 0}, log);

    m_middle      .m_slider.setValue(1.0f, false);
    m_middleExpand.m_slider.setValue(1.0f, false);
}

void ControlBoard::addParLog(const char *log)
{
    fflassert(str_haschar(log));
    m_logBoard.addParXML(m_logBoard.parCount(), {0, 0, 0, 0}, log);

    m_middle      .m_slider.setValue(1.0f, false);
    m_middleExpand.m_slider.setValue(1.0f, false);
}

void ControlBoard::addLog(int logType, const char *log)
{
    if(!log){
        throw fflerror("null log string");
    }

    switch(logType){
        case CBLOG_ERR:
            {
                g_log->addLog(LOGTYPE_WARNING, "%s", log);
                break;
            }
        default:
            {
                g_log->addLog(LOGTYPE_INFO, "%s", log);
                break;
            }
    }

    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    const char *xmlString = [logType]() -> const char *
    {
        // use hex to give alpha
        // color::String2Color has no alpha component

        switch(logType){
            case CBLOG_SYS: return "<par bgcolor = \"rgb(0x00, 0x80, 0x00)\"></par>";
            case CBLOG_DBG: return "<par bgcolor = \"rgb(0x00, 0x00, 0xff)\"></par>";
            case CBLOG_ERR: return "<par bgcolor = \"rgb(0xff, 0x00, 0x00)\"></par>";
            case CBLOG_DEF:
            default       : return "<par></par>";
        }
    }();

    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml template failed: %s", xmlString);
    }

    // to support <, >, / in xml string
    // don't directly pass the raw string to addParXML
    xmlDoc.RootElement()->SetText(log);

    tinyxml2::XMLPrinter printer;
    xmlDoc.Print(&printer);
    m_logBoard.addParXML(m_logBoard.parCount(), {0, 0, 0, 0}, printer.CStr());

    m_middle      .m_slider.setValue(1.0f, false);
    m_middleExpand.m_slider.setValue(1.0f, false);
}

TritexButton *ControlBoard::getButton(const std::string_view &buttonName)
{
    if     (buttonName == "Inventory"    ){ return &m_right.m_buttonInventory    ; }
    else if(buttonName == "HeroState"    ){ return &m_right.m_buttonHeroState    ; }
    else if(buttonName == "HeroMagic"    ){ return &m_right.m_buttonHeroMagic    ; }
    else if(buttonName == "Guild"        ){ return &m_right.m_buttonGuild        ; }
    else if(buttonName == "Team"         ){ return &m_right.m_buttonTeam         ; }
    else if(buttonName == "Quest"        ){ return &m_right.m_buttonQuest        ; }
    else if(buttonName == "Horse"        ){ return &m_right.m_buttonHorse        ; }
    else if(buttonName == "RuntimeConfig"){ return &m_right.m_buttonRuntimeConfig; }
    else if(buttonName == "FriendChat"   ){ return &m_right.m_buttonFriendChat   ; }
    else                                  { return nullptr                       ; }
}

void ControlBoard::onClickSwitchModeButton(int)
{
    if(m_expand){
        m_expand = false;
        m_middle.afterResize();
    }
    else{
        m_expand = true;
        m_middleExpand.afterResize();
    }
}

void ControlBoard::onInputDone()
{
    if(!m_cmdBoard.hasToken()){
        return;
    }

    const std::string fullXML = m_cmdBoard.getXML();
    const std::string fullStr = str_trim(m_cmdBoard.getText(), true, false);

    m_cmdBoard.clear();
    m_cmdBoard.setFocus(false);

    if(m_middle.show()){
        m_middle.onCmdCR();
    }
    else if(m_middleExpand.show()){
        m_middleExpand.onCmdCR();
    }

    switch(fullStr[0]){
        case '!': // broadcast
            {
                break;
            }
        case '@': // user command
            {
                if(m_processRun){
                    m_processRun->userCommand(fullStr.c_str() + 1);
                }
                break;
            }
        case '$': // lua command for super user
            {
                if(m_processRun){
                    m_processRun->luaCommand(fullStr.c_str() + 1);
                }
                break;
            }
        default: // normal talk
            {
                addXMLLog(fullXML.c_str());
                break;
            }
    }
}

int ControlBoard::shiftHeight() const
{
    return m_minimize ? 0 : CBMiddle::CB_MIDDLE_TEX_HEIGHT;
}

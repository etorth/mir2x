#include <type_traits>
#include "strf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "controlboard.hpp"
#include "queststateboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

QuestStateBoard::QuestStateBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          argX,
          argY,
          0,
          0,
          {},

          widgetPtr,
          autoDelete
      }

    , m_despBoard
      {
          DIR_UPLEFT,

          m_despX,
          m_despY,
          m_despW,

          nullptr,
          0,

          {0, 5, 0, 0},
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
          0,
          0,

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          nullptr,
          [this](const std::unordered_map<std::string, std::string> &attrList, int event)
          {
              if(event == BEVENT_RELEASE){
                  if(const auto id = LayoutBoard::findAttrValue(attrList, "id", nullptr)){
                      m_questDesp.at(id).folded = !m_questDesp.at(id).folded;
                      m_loadRequested = true;
                  }
              }
          },
          this,
      }

    , m_slider
      {
          DIR_UPLEFT,
          326,
          160,
          9,
          214,

          false,
          3,
          nullptr,

          this,
          false,
      }

    , m_lrButton
      {
          DIR_UPLEFT,
          315,
          76,
          {0X00000300, 0X00000300, 0X00000302},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_closeButton
      {
          DIR_UPLEFT,
          316,
          108,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
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
          true,

          this,
          false,
      }
    , m_processRun(runPtr)
{
    setShow(false);
    if(auto texPtr = g_progUseDB->retrieve(0X00000350)){
        std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid quest status board frame texture");
    }
}

void QuestStateBoard::update(double fUpdateTime)
{
    if(m_loadRequested){
        m_loadRequested = false;
        loadQuestDesp();
    }

    m_despBoard.update(fUpdateTime);
}

void QuestStateBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X00000350)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }

    m_despBoard.draw();

    m_slider.draw();
    m_lrButton.draw();
    m_closeButton.draw();
}

bool QuestStateBoard::processEventDefault(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_despBoard.processEvent(event, valid)){
        return true;
    }

    if(m_slider.processEvent(event, valid)){
        return true;
    }

    if(m_lrButton.processEvent(event, valid)){
        return true;
    }

    if(m_closeButton.processEvent(event, valid)){
        return true;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        {
                            setShow(false);
                            setFocus(false);
                            return true;
                        }
                    default:
                        {
                            return consumeFocus(false);
                        }
                }
            }
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y) || focus())){
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, x() + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, y() + event.motion.yrel));
                    moveBy(newX - x(), newY - y());
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                return consumeFocus(true);
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void QuestStateBoard::updateQuestDesp(SDQuestDespUpdate sdQDU)
{
    if(sdQDU.desp.has_value()){
        m_questDesp[sdQDU.name].desp[sdQDU.fsm] = sdQDU.desp.value();
    }
    else if(sdQDU.fsm == SYS_QSTFSM){
        m_questDesp.erase(sdQDU.name);
    }
    else{
        m_questDesp[sdQDU.name].desp.erase(sdQDU.fsm);
    }

    if(!show()){
        dynamic_cast<ControlBoard *>(m_processRun->getWidget("ControlBoard"))->getButton("Quest")->setBlinkTime(100, 100);
    }

    loadQuestDesp();
}

void QuestStateBoard::setQuestDesp(SDQuestDespList sdQDL)
{
    m_questDesp.clear();
    for(const auto &[quest, desps]: sdQDL){
        for(const auto &[fsm, desp]: desps){
            m_questDesp[quest].desp[fsm] = desp;
        }
    }

    loadQuestDesp();
}

void QuestStateBoard::loadQuestDesp()
{
    std::vector<std::string> xmlStrs;
    xmlStrs.push_back("<layout>");

    for(const auto &[quest, state]: m_questDesp){
        xmlStrs.push_back(str_printf(R"###(<par><event id="%s">%s</event></par>)###", quest.c_str(), quest.c_str()));
        if(!state.folded){
            xmlStrs.push_back(str_printf("<par>    %s</par>", (state.desp.count(SYS_QSTFSM) && str_haschar(state.desp.at(SYS_QSTFSM))) ? state.desp.at(SYS_QSTFSM).c_str() : "暂无任务描述"));
            for(const auto &[fsm, desp]: state.desp){
                if(fsm != SYS_QSTFSM){
                    xmlStrs.push_back(str_printf("<par>    * %s</par>", fsm.c_str()));
                    xmlStrs.push_back(str_printf("<par>      %s</par>", str_haschar(desp) ? desp.c_str() : "暂无任务描述"));
                }
            }
        }
    }

    xmlStrs.push_back("</layout>");

    m_despBoard.clear();
    m_despBoard.loadXML(str_join(xmlStrs, '\n').c_str());
}

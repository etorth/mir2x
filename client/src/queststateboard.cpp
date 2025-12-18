#include <type_traits>
#include "strf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "gui/controlboard/controlboard.hpp"
#include "queststateboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

QuestStateBoard::QuestStateBoard(
        dir8_t argDir,

        int argX,
        int argY,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,
          .w = {},
          .h = {},

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_bg
      {{
          .texLoadFunc = []
          {
              return g_progUseDB->retrieve(0X00000350);
          },
          .parent{this},
      }}

    , m_despBoard
      {{
          .x = m_despX,
          .y = m_despY,

          .lineWidth = m_despW,
          .margin
          {
              .down = 5,
          },

          .font
          {
              .id = 1,
              .size = 12,
          },

          .lineAlign = LALIGN_JUSTIFY,

          .onClickText = [this](const std::unordered_map<std::string, std::string> &attrList, int event)
          {
              if(event == BEVENT_RELEASE){
                  if(const auto id = LayoutBoard::findAttrValue(attrList, "id", nullptr)){
                      m_questDesp.at(id).folded = !m_questDesp.at(id).folded;
                      m_loadRequested = true;
                  }
              }
          },

          .parent{this},
      }}

    , m_slider
      {{
          .bar
          {
              .x = 326,
              .y = 160,
              .w = 9,
              .h = 214,
              .v = true,
          },

          .index = 3,
          .parent{this},
      }}

    , m_lrButton
      {{
          .x = 315,
          .y = 76,

          .texIDList
          {
              .off  = 0X00000300,
              .on   = 0X00000300,
              .down = 0X00000302,
          },

          .parent{this},
      }}

    , m_closeButton
      {{
          .x = 316,
          .y = 108,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
          },

          .parent{this},
      }}
{
    setShow(false);
}

void QuestStateBoard::updateDefault(double fUpdateTime)
{
    if(m_loadRequested){
        m_loadRequested = false;
        loadQuestDesp();
    }

    m_despBoard.update(fUpdateTime);
}

bool QuestStateBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_despBoard  .processEventParent(event, valid, m)){ return true; }
    if(m_slider     .processEventParent(event, valid, m)){ return true; }
    if(m_lrButton   .processEventParent(event, valid, m)){ return true; }
    if(m_closeButton.processEventParent(event, valid, m)){ return true; }

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
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || focus())){
                    const auto remapXDiff = m.x - m.ro->x;
                    const auto remapYDiff = m.y - m.ro->y;

                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + event.motion.yrel));

                    moveBy(newX - remapXDiff, newY - remapYDiff);
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

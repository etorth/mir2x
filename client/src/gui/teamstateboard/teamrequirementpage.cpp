#include <utility>
#include "colorf.hpp"
#include "totype.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "teamstateboard.hpp"
#include "teamrequirementpage.hpp"

extern SDLDevice *g_sdlDevice;
TeamRequirementPage::TeamRequirementPage(TeamStateBoard *board, Widget::VarBool argShow, Widget::VarBool argEditable, const char8_t *confirmLabel, std::function<void(const SDTeamRequirement &)> onConfirm)
    : Widget
      {{
          .x = TeamStateBoard::m_pageX,
          .y = TeamStateBoard::m_pageY,
          .w = TeamStateBoard::m_pageW,
          .h = TeamStateBoard::m_pageH,

          .attrs
          {
              .inst
              {
                  .show = std::move(argShow),
                  .moveOnFocus = false,
              },
          },
          .parent{board},
      }}

    , m_board(board)
    , m_editable(std::move(argEditable))
    , m_onConfirm(std::move(onConfirm))

    , m_mainPage
      {{
          .w = TeamStateBoard::m_pageW,
          .h = TeamStateBoard::m_pageH,

          .attrs
          {
              .inst
              {
                  .show = [this]{ return !m_jobIndex.has_value(); },
                  .moveOnFocus = false,
              },
          },
          .parent{this},
      }}

    , m_maxMember
      {{
          .dir = DIR_LEFT,
          .y = m_rowH / 2,
          .labelFirst = u8"最大人数",
          .align = DIR_NONE,
          .inputSize{40, 20},
          .parent{&m_mainPage},
      }}

    , m_autoAccept
      {{
          .dir = DIR_LEFT,
          .y = m_autoAcceptY + m_rowH / 2,

          .label{.text = u8"满足条件自动加入"},
          .getter = [this]{ return m_requirement.autoAccept; },
          .setter = [this](bool value){ m_requirement.autoAccept = value; },

          .attrs
          {
              .active = [this]{ return editable(); },
          },

          .parent{&m_mainPage},
      }}

    , m_confirm
      {{
          .y = m_confirmY,

          .textFunc = to_cstr(confirmLabel),
          .onTrigger = [this](Widget *, int)
          {
              applyInput();
          },

          .attrs
          {
              .show = [this]{ return editable(); },
          },

          .parent{&m_mainPage},
      }}
{
    TeamJobPanel::setupNumberInput(m_maxMember);
    m_maxMember.setActive([this]{ return editable(); });

    for(size_t i = 0; i < SDTeamRequirement::jobInfoList.size(); ++i){
        new TeamJobRow
        {{
            .y = m_jobRowY + to_d(i) * m_rowH,
            .w = TeamStateBoard::m_pageW,
            .h = m_rowH,

            .jobIndex = i,
            .requirement = &m_requirement.jobList[i],

            .onSetup = [this, i]{ m_jobIndex = i; },

            .editable = [this]{ return editable(); },
            .parent{&m_mainPage, true},
        }};

        m_jobPanelList[i] = new TeamJobPanel
        {{
            .w = TeamStateBoard::m_pageW,
            .h = TeamStateBoard::m_pageH,

            .jobIndex = i,
            .requirement = &m_requirement.jobList[i],

            .onBack = [this]{ showMainPage(); },

            .show = [this, i]{ return m_jobIndex == i; },
            .editable = [this]{ return editable(); },

            .parent{this, true},
        }};
    }
}

void TeamRequirementPage::loadRequirement(const SDTeamRequirement &requirement)
{
    m_requirement = requirement;
    m_maxMember.inputLine()->setInput((m_requirement.maxMember > 0) ? std::to_string(m_requirement.maxMember).c_str() : "");

    for(auto panel: m_jobPanelList){
        panel->loadInput();
    }
}

void TeamRequirementPage::applyInput()
{
    m_requirement.maxMember = TeamJobPanel::parseNumberInput(m_maxMember);
    for(auto panel: m_jobPanelList){
        panel->applyInput();
    }

    if(m_requirement.maxMember == 1 || m_requirement.maxMember > SYS_MAXTEAMMEMBER){
        m_board->m_processRun->addCBLog(CBLOG_ERR, u8"最大人数需在2-%d之间", SYS_MAXTEAMMEMBER);
        return;
    }

    if(!m_requirement.anyJobAllowed()){
        m_board->m_processRun->addCBLog(CBLOG_ERR, u8"至少需要允许一个职业");
        return;
    }

    for(size_t i = 0; i < m_requirement.jobList.size(); ++i){
        if(const auto &jobReq = m_requirement.jobList[i]; jobReq.allow && jobReq.minLevel > 0 && jobReq.maxLevel > 0 && jobReq.minLevel > jobReq.maxLevel){
            m_board->m_processRun->addCBLog(CBLOG_ERR, u8"%s：最低等级不能高于最高等级", to_cstr(SDTeamRequirement::jobInfoList[i].name));
            return;
        }
    }

    if(!m_requirement.valid()){
        m_board->m_processRun->addCBLog(CBLOG_ERR, u8"队伍要求设置无效");
        return;
    }

    if(m_onConfirm){
        m_onConfirm(m_requirement);
    }
}

#include <algorithm>
#include <format>
#include <utility>
#include "jobf.hpp"
#include "totype.hpp"
#include "hero.hpp"
#include "myhero.hpp"
#include "processrun.hpp"
#include "gui/skillboard/skillboard.hpp"
#include "teamstateboard.hpp"
#include "teamnearbypage.hpp"

TeamNearbyPage::TeamNearbyPage(TeamStateBoard *board, Widget::VarBool argShow)
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

    , m_onlyMatchCheck
      {{
          .label{.text = u8"仅显示满足条件的队伍"},

          .getter = [this]{ return m_onlyMatch; },
          .setter = [this](bool value)
          {
              m_onlyMatch = value;
              updateViewList();
          },

          .parent{this},
      }}

    , m_listView
      {{
          .y = 24,
          .w = TeamStateBoard::m_pageW,
          .rowCount = 12,

          .columnList
          {
              {.title = u8"队长", .w = 150, .text = [this](size_t i){ return viewTeam(i).name; }},
              {.title = u8"职业", .w =  60, .text = [this](size_t i){ return std::string(to_cstr(jobf::jobName(viewTeam(i).job).at(0))); }},
              {.title = u8"等级", .w =  60, .text = [this](size_t i){ return std::to_string(viewTeam(i).level); }},
              {.title = u8"人数", .w =  70, .text = [this](size_t i)
              {
                  const auto &team = viewTeam(i);
                  return std::format("{}/{}", team.memberCount, team.requirement.memberLimit());
              }},
          },

          .buttonList
          {
              {.label = u8"申请", .onClick = [this](size_t i)
              {
                  m_board->m_processRun->requestJoinTeam(viewTeam(i).leader);
                  m_board->m_processRun->addCBLog(CBLOG_SYS, u8"已申请加入%s的队伍", viewTeam(i).name.c_str());
              }},
          },

          .size = [this]{ return m_viewList.size(); },
          .parent{this},
      }}

    , m_refresh
      {{
          .x = 0,
          .y = TeamStateBoard::m_pageH - 28,

          .textFunc = to_cstr(u8"刷新"),
          .onTrigger = [this](Widget *, int)
          {
              queryTeamList();
          },

          .parent{this},
      }}

    , m_pickPlayer
      {{
          .dir = DIR_UPRIGHT,
          .x = TeamStateBoard::m_pageW,
          .y = TeamStateBoard::m_pageH - 28,

          .textFunc = to_cstr(u8"点选玩家"),
          .onTrigger = [this](Widget *, int)
          {
              m_board->m_processRun->setCursor(ProcessRun::CURSOR_TEAMFLAG);
              m_board->setShow(false);
          },

          .parent{this},
      }}
{
    updateViewList();
}

void TeamNearbyPage::updateDefault(double fUpdateTime)
{
    Widget::updateDefault(fUpdateTime);

    const bool visible = m_board->show() && show();
    if(visible && !m_visible){
        queryTeamList();
    }

    m_visible = visible;
    if(m_onlyMatch && visible){
        updateViewList(); // hero level/stat/magic may change
    }
}

void TeamNearbyPage::queryTeamList()
{
    m_teamList.clear();
    updateViewList();

    const auto myHeroUID = m_board->m_processRun->getMyHeroUID();
    for(const auto &[uid, coPtr]: m_board->m_processRun->getCOList()){
        if(uid == myHeroUID){
            continue;
        }

        if(const auto hero = dynamic_cast<const Hero *>(coPtr.get()); hero && hero->isTeamLeader()){
            m_board->m_processRun->requestQueryTeamInfo(uid);
        }
    }
}

void TeamNearbyPage::setTeamInfo(SDTeamInfo sdTI)
{
    if(auto p = std::ranges::find(m_teamList, sdTI.leader, &SDTeamInfo::leader); p != m_teamList.end()){
        *p = std::move(sdTI);
    }
    else{
        m_teamList.push_back(std::move(sdTI));
    }
    updateViewList();
}

void TeamNearbyPage::updateViewList()
{
    m_viewList.clear();
    const auto myHero = m_board->m_processRun->getMyHero();

    if(!m_onlyMatch || !myHero){
        for(size_t i = 0; i < m_teamList.size(); ++i){
            m_viewList.push_back(i);
        }
        return;
    }

    const int job = myHero->job();
    const int level = to_d(myHero->getLevel());

    const int stat = [&myHero, job]
    {
        const auto combatNode = myHero->getCombatNode();
        if(job & JOB_WARRIOR) return combatNode.dc[1];
        if(job & JOB_WIZARD ) return combatNode.mc[1];
        if(job & JOB_TAOIST ) return combatNode.sc[1];
        return 0;
    }();

    const auto skillBoard = dynamic_cast<const SkillBoard *>(m_board->m_processRun->getWidget("SkillBoard"));
    const auto hasMagic = [skillBoard](uint32_t magicID)
    {
        return skillBoard && skillBoard->getConfig().hasMagicID(magicID);
    };

    for(size_t i = 0; i < m_teamList.size(); ++i){
        const auto &team = m_teamList[i];
        if(team.memberCount >= team.requirement.memberLimit()){
            continue;
        }

        if(team.requirement.accept(job, level, stat, hasMagic)){
            m_viewList.push_back(i);
        }
    }
}

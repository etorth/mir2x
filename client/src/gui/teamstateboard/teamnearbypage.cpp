#include <format>
#include <utility>
#include "jobf.hpp"
#include "totype.hpp"
#include "myhero.hpp"
#include "processrun.hpp"
#include "gui/skillboard/skillboard.hpp"
#include "teamstateboard.hpp"
#include "teamnearbypage.hpp"

static TeamRequirement sampleRequirement(int maxMember, TeamJobRequirement warrior, TeamJobRequirement wizard, TeamJobRequirement taoist, bool autoAccept = false)
{
    return TeamRequirement
    {
        .maxMember = maxMember,
        .autoAccept = autoAccept,
        .jobList{std::move(warrior), std::move(wizard), std::move(taoist)},
    };
}

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

      // sample data, server doesn't report nearby teams yet
    , m_teamList
      {
          {.leader = "冷月无声", .job = JOB_WARRIOR, .level = 42, .memberCount = 5, .requirement = sampleRequirement(10, {.minLevel = 35}, {.minLevel = 35}, {.minLevel = 35})},
          {.leader = "逍遥子"  , .job = JOB_TAOIST , .level = 38, .memberCount = 3, .requirement = sampleRequirement( 8, {.allow = false}, {}, {})},
          {.leader = "风之谷"  , .job = JOB_WIZARD , .level = 45, .memberCount = 9, .requirement = sampleRequirement(10, {.minLevel = 40, .maxLevel = 50}, {.minLevel = 40, .maxLevel = 50}, {.allow = false})},
          {.leader = "无名剑客", .job = JOB_WARRIOR, .level = 22, .memberCount = 2},
          {.leader = "夜行者"  , .job = JOB_TAOIST , .level = 31, .memberCount = 6, .requirement = sampleRequirement(10, {.minLevel = 25}, {.minLevel = 25}, {.minLevel = 25, .minStat = 10})},
          {.leader = "天下第一", .job = JOB_WIZARD , .level = 50, .memberCount = 1, .requirement = sampleRequirement( 0, {.minLevel = 45}, {.minLevel = 45}, {.minLevel = 45}, true)},
      }

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
              {.title = u8"队长", .w = 150, .text = [this](size_t i){ return viewTeam(i).leader; }},
              {.title = u8"职业", .w =  60, .text = [this](size_t i){ return std::string(to_cstr(jobf::jobName(viewTeam(i).job).at(0))); }},
              {.title = u8"等级", .w =  60, .text = [this](size_t i){ return std::to_string(viewTeam(i).level); }},
              {.title = u8"人数", .w =  70, .text = [this](size_t i)
              {
                  const auto &team = viewTeam(i);
                  return (team.requirement.maxMember > 0) ? std::format("{}/{}", team.memberCount, team.requirement.maxMember) : std::to_string(team.memberCount);
              }},
          },

          .buttonList
          {
              {.label = u8"申请", .onClick = [this](size_t i)
              {
                  m_board->m_processRun->addCBLog(CBLOG_SYS, u8"申请加入%s的队伍：暂未实现", viewTeam(i).leader.c_str());
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
              m_board->m_processRun->addCBLog(CBLOG_SYS, u8"刷新附近队伍：暂未实现");
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
    if(m_onlyMatch && show()){
        updateViewList(); // hero level/stat/magic may change
    }
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
        if(team.requirement.maxMember > 0 && team.memberCount >= team.requirement.maxMember){
            continue;
        }

        if(team.requirement.accept(job, level, stat, hasMagic)){
            m_viewList.push_back(i);
        }
    }
}

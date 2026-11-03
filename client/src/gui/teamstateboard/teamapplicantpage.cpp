#include <format>
#include <utility>
#include "jobf.hpp"
#include "totype.hpp"
#include "processrun.hpp"
#include "teamstateboard.hpp"
#include "teamapplicantpage.hpp"

TeamApplicantPage::TeamApplicantPage(TeamStateBoard *board, Widget::VarBool argShow)
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

    , m_title
      {{
          .textFunc = [this]
          {
              return std::format("申请加入的玩家：{}", m_board->m_teamCandidateList.size());
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
              {.title = u8"名字", .w = 120, .text = [this](size_t i){ return m_board->m_teamCandidateList.at(i).first.player.name; }},
              {.title = u8"职业", .w =  60, .text = [this](size_t i){ return std::string(to_cstr(jobf::jobName(m_board->m_teamCandidateList.at(i).first.player.job).at(0))); }},
              {.title = u8"等级", .w =  60, .text = [this](size_t i){ return std::to_string(m_board->m_teamCandidateList.at(i).first.player.level); }},
              {.title = u8"时间", .w =  90, .text = [this](size_t i){ return waitTimeString(m_board->m_teamCandidateList.at(i).second.diff_sec()); }},
          },

          .buttonList
          {
              {.label = u8"同意", .onClick = [this](size_t i)
              {
                  m_board->m_processRun->requestJoinTeam(m_board->m_teamCandidateList.at(i).first.player.uid);
                  m_board->m_teamCandidateList.erase(m_board->m_teamCandidateList.begin() + i);
              }},

              {.label = u8"拒绝", .onClick = [this](size_t i)
              {
                  m_board->m_teamCandidateList.erase(m_board->m_teamCandidateList.begin() + i);
              }},
          },

          .size = [this]{ return m_board->m_teamCandidateList.size(); },
          .parent{this},
      }}

    , m_clear
      {{
          .dir = DIR_UPRIGHT,
          .x = TeamStateBoard::m_pageW,
          .y = TeamStateBoard::m_pageH - 28,

          .textFunc = to_cstr(u8"全部拒绝"),
          .onTrigger = [this](Widget *, int)
          {
              m_board->m_teamCandidateList.clear();
          },

          .attrs
          {
              .show = [this]{ return !m_board->m_teamCandidateList.empty(); },
          },
          .parent{this},
      }}
{}

std::string TeamApplicantPage::waitTimeString(uint64_t sec)
{
    if(sec < 60){
        return std::format("{}秒前", sec);
    }
    else if(sec < 60 * 60){
        return std::format("{}分钟前", sec / 60);
    }
    else{
        return std::format("{}小时前", sec / (60 * 60));
    }
}

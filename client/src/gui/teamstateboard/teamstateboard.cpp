#include <format>
#include <utility>
#include <algorithm>
#include "totype.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "teamstateboard.hpp"

extern SDLDevice *g_sdlDevice;
TeamStateBoard::TeamStateBoard(int argX, int argY, ProcessRun *runPtr, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .x = argX,
          .y = argY,
          .w = m_boardW,
          .h = m_boardH,

          .attrs
          {
              .inst
              {
                  .moveOnFocus = false,
              },
          },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(fflcheck(runPtr))

    , m_frameBoard
      {{
          .w = m_boardW,
          .h = m_boardH,
          .parent{this},
      }}

    , m_leftMenuBackground
      {{
          .x = m_menuX,
          .y = m_menuY,
          .w = m_menuW,
          .h = m_boardH - m_menuY * 2,

          .drawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(                             colorf::A_SHF(128), drawDstX, drawDstY, self->w(), self->h(), 10);
              g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(100), drawDstX, drawDstY, self->w(), self->h(), 10);
          },

          .parent{this},
      }}

      // only 3 tabs show at same time
      // tabs for in-team and not-in-team share the same slots

    , m_tabNearby
      {{
          .x = m_menuX + 5,
          .y = m_menuY + 10 + 0 * (m_tabH + m_tabGap),
          .w = m_menuW - 10,
          .h = m_tabH,

          .label = to_cstr(u8"附近队伍"),
          .selected = [this]{ return currentPage() == PAGE_NEARBY; },
          .onClick = [this]{ m_page = PAGE_NEARBY; },

          .attrs
          {
              .show = [this]{ return pageVisible(PAGE_NEARBY); },
          },
          .parent{this},
      }}

    , m_tabCreate
      {{
          .x = m_menuX + 5,
          .y = m_menuY + 10 + 1 * (m_tabH + m_tabGap),
          .w = m_menuW - 10,
          .h = m_tabH,

          .label = to_cstr(u8"创建队伍"),
          .selected = [this]{ return currentPage() == PAGE_CREATE; },
          .onClick = [this]{ m_page = PAGE_CREATE; m_pageCreate.showMainPage(); },

          .attrs
          {
              .show = [this]{ return pageVisible(PAGE_CREATE); },
          },
          .parent{this},
      }}

    , m_tabMember
      {{
          .x = m_menuX + 5,
          .y = m_menuY + 10 + 0 * (m_tabH + m_tabGap),
          .w = m_menuW - 10,
          .h = m_tabH,

          .label = to_cstr(u8"队伍成员"),
          .selected = [this]{ return currentPage() == PAGE_MEMBER; },
          .onClick = [this]{ m_page = PAGE_MEMBER; },

          .attrs
          {
              .show = [this]{ return pageVisible(PAGE_MEMBER); },
          },
          .parent{this},
      }}

    , m_tabSetting
      {{
          .x = m_menuX + 5,
          .y = m_menuY + 10 + 1 * (m_tabH + m_tabGap),
          .w = m_menuW - 10,
          .h = m_tabH,

          .label = to_cstr(u8"队伍设置"),
          .selected = [this]{ return currentPage() == PAGE_SETTING; },
          .onClick = [this]{ m_page = PAGE_SETTING; m_pageSetting.showMainPage(); },

          .attrs
          {
              .show = [this]{ return pageVisible(PAGE_SETTING); },
          },
          .parent{this},
      }}

    , m_tabApplicant
      {{
          .x = m_menuX + 5,
          .y = m_menuY + 10 + 2 * (m_tabH + m_tabGap),
          .w = m_menuW - 10,
          .h = m_tabH,

          .label = [this]
          {
              if(m_teamCandidateList.empty()){
                  return std::string(to_cstr(u8"申请列表"));
              }
              return std::format("申请列表({})", m_teamCandidateList.size());
          },

          .selected = [this]{ return currentPage() == PAGE_APPLICANT; },
          .onClick = [this]{ m_page = PAGE_APPLICANT; },

          .parent{this},
      }}

    , m_pageNearby(this, [this]{ return currentPage() == PAGE_NEARBY; })
    , m_pageCreate(this, [this]{ return currentPage() == PAGE_CREATE; }, true, u8"创建队伍", [this](const SDTeamRequirement &requirement)
      {
          m_processRun->requestCreateTeam(requirement);
      })

    , m_pageMember(this, [this]{ return currentPage() == PAGE_MEMBER; })
    , m_pageSetting(this, [this]{ return currentPage() == PAGE_SETTING; }, [this]{ return isTeamLeader(); }, u8"保存设置", [this](const SDTeamRequirement &requirement)
      {
          m_processRun->requestSetTeamRequirement(requirement);
      })

    , m_pageApplicant(this, [this]{ return currentPage() == PAGE_APPLICANT; })
{
    setShow(false);
}

bool TeamStateBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(Widget::processEventDefault(event, valid, m)){
        return true;
    }

    switch(event.type){
        case SDL_EVENT_KEY_DOWN:
            {
                if(event.key.key == SDLK_ESCAPE){
                    setShow(false);
                    return consumeFocus(false);
                }
                return false;
            }
        case SDL_EVENT_MOUSE_MOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(to_d(event.motion.x), to_d(event.motion.y)) || focus())){
                    if(const auto par = parent()){
                        moveBy(to_d(event.motion.xrel), to_d(event.motion.yrel), par->roi());
                    }
                    else{
                        moveBy(to_d(event.motion.xrel), to_d(event.motion.yrel), Widget::makeROI(0, 0, g_sdlDevice->getWindowLogicalSize()));
                    }
                    return consumeFocus(true);
                }
                return false;
            }
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                return consumeFocus(m.in(to_d(event.button.x), to_d(event.button.y)));
            }
        case SDL_EVENT_MOUSE_WHEEL:
            {
                return m.in(to_d(event.wheel.mouse_x), to_d(event.wheel.mouse_y));
            }
        default:
            {
                return false;
            }
    }
}

void TeamStateBoard::addTeamCandidate(SDTeamCandidate sdTC)
{
    if(std::ranges::any_of(m_teamMemberList.memberList, [&sdTC](const auto &member){ return member.uid == sdTC.player.uid; })){
        return;
    }

    std::erase_if(m_teamCandidateList, [&sdTC](const auto &candidate)
    {
        return candidate.first.player.uid == sdTC.player.uid;
    });

    m_teamCandidateList.emplace_front(std::move(sdTC), hres_timer());
}

void TeamStateBoard::setTeamMemberList(SDTeamMemberList sdTML)
{
    const bool requirementChanged = (sdTML.requirement != m_teamMemberList.requirement);
    m_teamMemberList = std::move(sdTML);

    if(requirementChanged){
        m_pageSetting.loadRequirement(m_teamMemberList.requirement);
    }
    std::erase_if(m_teamCandidateList, [this](const auto &candidate)
    {
        return std::ranges::any_of(m_teamMemberList.memberList, [&candidate](const auto &member)
        {
            return member.uid == candidate.first.player.uid;
        });
    });
}

bool TeamStateBoard::inTeam() const
{
    return std::ranges::any_of(m_teamMemberList.memberList, [this](const auto &member)
    {
        return member.uid == m_processRun->getMyHeroUID();
    });
}

bool TeamStateBoard::isTeamLeader() const
{
    return inTeam() && (m_teamMemberList.teamLeader == m_processRun->getMyHeroUID());
}

bool TeamStateBoard::pageVisible(PageType page) const
{
    switch(page){
        case PAGE_NEARBY:
        case PAGE_CREATE : return !inTeam();
        case PAGE_MEMBER :
        case PAGE_SETTING: return  inTeam();
        default          : return  true;
    }
}

TeamStateBoard::PageType TeamStateBoard::currentPage() const
{
    if(pageVisible(m_page)){
        return m_page;
    }
    return inTeam() ? PAGE_MEMBER : PAGE_NEARBY;
}

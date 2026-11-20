#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include "xmlf.hpp"
#include "totype.hpp"
#include "colorf.hpp"
#include "sysconst.hpp"
#include "protocoldef.hpp"
#include "sdldevice.hpp"
#include "progusedb.hpp"
#include "processrun.hpp"
#include "inputstringboard.hpp"
#include "guildboard.hpp"

extern ProgUseDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

static std::string noticeXML(const std::string &notice)
{
    std::string xml = "<layout>";
    for(size_t begin = 0;;){
        const auto end = notice.find('\n', begin);
        xml += xmlf::toParString("%s", notice.substr(begin, end - begin).c_str());

        if(end == std::string::npos){
            break;
        }
        begin = end + 1;
    }
    return xml + "</layout>";
}

GuildBoard::GuildBoard(int argX, int argY, ProcessRun *runPtr, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .x = argX,
          .y = argY,
          .w = 594,
          .h = 444,

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

    , m_processRun(runPtr)
    , m_bg
      {{
          .texLoadFunc = [](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00000500);
          },

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_guildName
      {{
          .dir = DIR_NONE,
          .x = 251,
          .y = 20,

          .textFunc = [this]() -> std::string
          {
              return m_guild.name;
          },

          .font
          {
              .id = 1,
              .size = 12,
          },

          .parent{this},
      }}

    , m_noGuild
      {{
          .dir = DIR_NONE,
          .x = m_textX + m_textW / 2,
          .y = 200,

          .textFunc = to_cstr(u8"你还没有加入行会"),
          .font
          {
              .id = 1,
              .size = 12,
          },

          .attrs
          {
              .show = [this]{ return m_guild.guildID == 0; },
          },
          .parent{this},
      }}

    , m_noticeView
      {{
          .x = m_textX,
          .y = m_textY,

          .getter = [this]() -> Widget * { return &m_noticeBoard; },
          .vr
          {
              0,
              [this]{ return scrollOffset(m_noticeBoard.h() - m_noticeH); },
              m_textW,
              m_noticeH,
          },

          .attrs
          {
              .inst
              {
                  .show = [this]{ return pageVisible(PAGE_NOTICE) && !m_editNotice; },
              },
          },
          .parent{this},
      }}

    , m_noticeBoard
      {{
          .lineWidth = m_textW,
          .font
          {
              .id = 1,
              .size = 12,
          },
          .parent{&m_noticeView},
      }}

    , m_noticeEditFrame
      {{
          .x = m_textX - 3,
          .y = m_textY - 3,
          .w = m_textW + 6,
          .h = m_noticeH + 6,

          .drawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(16), drawDstX, drawDstY, self->w(), self->h());
              g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), drawDstX, drawDstY, self->w(), self->h());
          },

          .attrs
          {
              .show = [this]{ return pageVisible(PAGE_NOTICE) && m_editNotice; },
          },
          .parent{this},
      }}

    , m_noticeEditView
      {{
          .x = m_textX,
          .y = m_textY,

          .getter = [this]() -> Widget * { return &m_noticeEditBoard; },
          .vr
          {
              0,
              [this]{ return scrollOffset(m_noticeEditBoard.h() - m_noticeH); },
              m_textW,
              m_noticeH,
          },

          .attrs
          {
              .inst
              {
                  .show = [this]{ return pageVisible(PAGE_NOTICE) && m_editNotice; },
              },
          },
          .parent{this},
      }}

    , m_noticeEditBoard
      {{
          .lineWidth = m_textW,
          .canEdit = true,
          .enableIME = [this]{ return m_processRun->getRuntimeConfig<RTCFG_IME>(); },

          .font
          {
              .id = 1,
              .size = 12,
          },

          .onCursorMove = [this]
          {
              if(const auto maxOffset = m_noticeEditBoard.h() - m_noticeH; maxOffset > 0){
                  const auto offset = scrollOffset(maxOffset);
                  const auto [_, cursorY, _, cursorH] = m_noticeEditBoard.getCursorPLoc();

                  if(cursorY < offset){
                      m_slider.setValue(to_f(cursorY) / maxOffset, false);
                  }
                  else if(cursorY + cursorH > offset + m_noticeH){
                      m_slider.setValue(to_f(cursorY + cursorH - m_noticeH) / maxOffset, false);
                  }
              }
          },

          .parent{&m_noticeEditView},
      }}

    , m_noticeEditHint
      {{
          .x = m_textX,
          .y = m_textY + m_noticeH + 6,

          .textFunc = to_cstr(u8"再次点击编辑按钮保存，按Esc取消"),
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::RGBA(220, 220, 220, 128),
          },

          .attrs
          {
              .show = [this]{ return pageVisible(PAGE_NOTICE) && m_editNotice; },
          },
          .parent{this},
      }}

    , m_memberList
      {{
          .x = m_memberX,
          .y = m_memberY,
          .w = m_memberW,
          .h = m_memberListH,

          .attrs
          {
              .inst
              {
                  .show = [this]{ return pageVisible(PAGE_MEMBER); },
                  .moveOnFocus = false,
                  .processEvent = [this](Widget *self, const SDL_Event &event, bool valid, Widget::ROIMap m)
                  {
                      if(!m.calibrate(self)){
                          return false;
                      }

                      if(!valid){
                          return self->consumeFocus(false);
                      }

                      if((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) && (event.button.button == SDL_BUTTON_LEFT) && m.in(to_d(event.button.x), to_d(event.button.y))){
                          const auto index = [&event, &m, this]() -> std::optional<size_t>
                          {
                              if(const auto rowY = to_d(event.button.y) - (m.y - m.ro->y) - m_memberHeaderH; rowY >= 0){
                                  return memberIndex(rowY / m_memberRowH);
                              }
                              return std::nullopt;
                          }();

                          m_selectedDBID = index.has_value() ? m_guild.memberList.at(index.value()).dbid : 0;
                          return self->consumeFocus(true);
                      }
                      return false;
                  },
              },
          },
          .parent{this},
      }}

    , m_invite
      {{
          .dir = DIR_UPRIGHT,
          .x = m_memberX + m_memberW - 6,
          .y = m_memberY + m_memberListH + 8,

          .textFunc = to_cstr(u8"邀请"),
          .font
          {
              .id = 1,
              .size = 12,
          },

          .onTrigger = [this](Widget *, int)
          {
              inviteMember();
          },

          .attrs
          {
              .show = [this]{ return pageVisible(PAGE_MEMBER); },
          },
          .parent{this},
      }}

    , m_chatView
      {{
          .x = m_textX,
          .y = m_textY,

          .getter = [this]() -> Widget * { return &m_chatBoard; },
          .vr
          {
              0,
              [this]{ return scrollOffset(m_chatBoard.h() - m_chatH); },
              m_textW,
              m_chatH,
          },

          .attrs
          {
              .inst
              {
                  .show = [this]{ return pageVisible(PAGE_CHAT); },
              },
          },
          .parent{this},
      }}

    , m_chatBoard
      {{
          .lineWidth = m_textW,
          .font
          {
              .id = 1,
              .size = 12,
          },
          .parent{&m_chatView},
      }}

    , m_chatInputBg
      {{
          .x = m_textX - 2,
          .y = m_chatInputY - 2,
          .w = m_textW + 4,
          .h = m_chatInputH + 4,

          .drawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(32), drawDstX, drawDstY, self->w(), self->h());
              g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), drawDstX, drawDstY, self->w(), self->h());
          },

          .attrs
          {
              .show = [this]{ return pageVisible(PAGE_CHAT); },
          },
          .parent{this},
      }}

    , m_chatInput
      {{
          .x = m_textX + 2,
          .y = m_chatInputY,
          .w = m_textW - 4,
          .h = m_chatInputH,

          .enableIME = [this]{ return m_processRun->getRuntimeConfig<RTCFG_IME>(); },
          .font
          {
              .id = 1,
              .size = 12,
          },

          .cursor
          {
              .align = DIR_LEFT,
          },

          .onCR = [this]
          {
              sendChat();
          },

          .validate = [](const std::string &currentInput, const std::string &newInput)
          {
              return currentInput.size() + newInput.size() < m_chatMessageSize;
          },

          .parent{this},
      }}

    , m_closeButton
      {{
          .x = 554,
          .y = 399,

          .texAniOpt = 0X0000001C,

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
              setFocus(false);
          },

          .parent{this},
      }}

    , m_announcement
      {{
          .x = 40,
          .y = 385,

          .texAniOpt = 0X00000510,

          .onTrigger = [this](Widget *, int)
          {
              setPage(PAGE_NOTICE);
          },

          .parent{this},
      }}

    , m_members
      {{
          .x = 90,
          .y = 385,

          .texAniOpt = 0X00000520,

          .onTrigger = [this](Widget *, int)
          {
              setPage(PAGE_MEMBER);
          },

          .parent{this},
      }}

    , m_chat
      {{
          .x = 140,
          .y = 385,

          .texAniOpt = 0X00000530,

          .onTrigger = [this](Widget *, int)
          {
              setPage(PAGE_CHAT);
          },

          .parent{this},
      }}

    , m_editAnnouncement
      {{
          .x = 290,
          .y = 385,

          .texAniOpt = 0X00000540,

          .onTrigger = [this](Widget *, int)
          {
              editNotice();
          },

          .parent{this},
      }}

    , m_removeMember
      {{
          .x = 340,
          .y = 385,

          .texAniOpt = 0X00000550,

          .onTrigger = [this](Widget *, int)
          {
              removeMember();
          },

          .parent{this},
      }}

    , m_disbandGuild
      {{
          .x = 390,
          .y = 385,

          .texAniOpt = 0X00000560,

          .onTrigger = [this](Widget *, int)
          {
              leaveGuild();
          },

          .parent{this},
      }}

    , m_editMemberPosition
      {{
          .x = 440,
          .y = 385,

          .texAniOpt = 0X00000570,

          .onTrigger = [this](Widget *, int)
          {
              editMemberPosition();
          },

          .parent{this},
      }}

    , m_dissolveCovenant
      {{
          .x = 490,
          .y = 385,

          .texAniOpt = 0X00000580,

          .onTrigger = [this](Widget *, int)
          {
              if(checkGuild()){
                  m_processRun->addCBLog(CBLOG_SYS, u8"行会联盟暂未开放");
              }
          },

          .parent{this},
      }}

    , m_slider
      {{
          .bar
          {
              .x = 564,
              .y = 53,
              .w = 9,
              .h = 294,
          },

          .index = 3,
          .parent{this},
      }}
{
    constexpr int titleX =  10;
    constexpr int nameX  = 170;
    constexpr int stateX = 420;

    new GfxShapeBoard
    {{
        .w = m_memberW,
        .h = m_memberListH,

        .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
        {
            g_sdlDevice->drawLine(colorf::RGBA(231, 231, 189, 80), drawDstX + 1, drawDstY + m_memberHeaderH, drawDstX + self->w() - 2, drawDstY + m_memberHeaderH);
            for(int row = 0; row < m_memberRowCount; ++row){
                if(const auto index = memberIndex(row)){
                    const int rowY = drawDstY + m_memberHeaderH + 1 + row * m_memberRowH;
                    if(m_guild.memberList.at(index.value()).dbid == m_selectedDBID){
                        g_sdlDevice->fillRectangle(colorf::RGBA(0, 80, 255, 96), drawDstX, rowY, self->w(), m_memberRowH);
                    }
                    else if(index.value() % 2){
                        g_sdlDevice->fillRectangle(colorf::RGBA(255, 255, 255, 16), drawDstX, rowY, self->w(), m_memberRowH);
                    }
                }
            }
        },

        .parent{&m_memberList, true},
    }};

    const std::pair<int, const char8_t *> headerList[]
    {
        {titleX, u8"职位"},
        {nameX , u8"名字"},
        {stateX, u8"状态"},
    };

    for(const auto &[headerX, header]: headerList){
        new TextBoard
        {{
            .dir = DIR_LEFT,
            .x = headerX,
            .y = m_memberHeaderH / 2,

            .textFunc = to_cstr(header),
            .font
            {
                .id = 1,
                .size = 12,
                .color = colorf::RGBA(231, 231, 189, 255),
            },

            .parent{&m_memberList, true},
        }};
    }

    for(int row = 0; row < m_memberRowCount; ++row){
        const int rowY = m_memberHeaderH + 1 + row * m_memberRowH + m_memberRowH / 2;
        const auto fontColor = [this, row]() -> uint32_t
        {
            if(const auto index = memberIndex(row); index.has_value() && !m_guild.memberList.at(index.value()).online){
                return colorf::GREY_A255;
            }
            return colorf::WHITE_A255;
        };

        new TextBoard
        {{
            .dir = DIR_LEFT,
            .x = titleX,
            .y = rowY,

            .textFunc = [this, row]() -> std::string
            {
                if(const auto index = memberIndex(row)){
                    if(const auto &member = m_guild.memberList.at(index.value()); member.title.empty()){
                        return to_cstr((member.rank == GUILDRANK_LEADER) ? u8"掌门人" : u8"成员");
                    }
                    else{
                        return member.title;
                    }
                }
                return {};
            },

            .font
            {
                .id = 1,
                .size = 12,
                .color = fontColor,
            },

            .parent{&m_memberList, true},
        }};

        new TextBoard
        {{
            .dir = DIR_LEFT,
            .x = nameX,
            .y = rowY,

            .textFunc = [this, row]() -> std::string
            {
                if(const auto index = memberIndex(row)){
                    return m_guild.memberList.at(index.value()).name;
                }
                return {};
            },

            .font
            {
                .id = 1,
                .size = 12,
                .color = fontColor,
            },

            .parent{&m_memberList, true},
        }};

        new TextBoard
        {{
            .dir = DIR_LEFT,
            .x = stateX,
            .y = rowY,

            .textFunc = [this, row]() -> std::string
            {
                if(const auto index = memberIndex(row)){
                    return to_cstr(m_guild.memberList.at(index.value()).online ? u8"在线" : u8"离线");
                }
                return {};
            },

            .font
            {
                .id = 1,
                .size = 12,
                .color = fontColor,
            },

            .parent{&m_memberList, true},
        }};
    }

    m_chatInput.setShow([this]{ return pageVisible(PAGE_CHAT); });
    setShow(false);
}

bool GuildBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    // focused LayoutBoard takes Esc to drop its focus, check before it
    if(m_editNotice && focus() && (event.type == SDL_EVENT_KEY_DOWN) && (event.key.key == SDLK_ESCAPE)){
        stopEditNotice();
        return true;
    }

    if(Widget::processEventDefault(event, valid, m)){
        return true;
    }

    // only clicks change focus here, other events must not drop the focus of the notice editor or the chat input
    switch(event.type){
        case SDL_EVENT_KEY_DOWN:
            {
                if(event.key.key == SDLK_ESCAPE){
                    setShow(false);
                    setFocus(false);
                    return true;
                }
                return false;
            }
        case SDL_EVENT_MOUSE_MOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(to_d(event.motion.x), to_d(event.motion.y)) || focus())){
                    const auto remapXDiff = m.x - m.ro->x;
                    const auto remapYDiff = m.y - m.ro->y;

                    const auto [winLogicalW, winLogicalH] = g_sdlDevice->getWindowLogicalSize();
                    const int maxX = winLogicalW - w();
                    const int maxY = winLogicalH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + to_d(event.motion.xrel)));
                    const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + to_d(event.motion.yrel)));

                    moveBy(newX - remapXDiff, newY - remapYDiff);
                    return consumeFocus(true);
                }
                return false;
            }
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                const int eventX = to_d(event.button.x);
                const int eventY = to_d(event.button.y);

                if(!m.in(eventX, eventY)){
                    return consumeFocus(false);
                }

                // the editing board can be shorter than the edit area, a click on the blank part still focuses it
                if(pageVisible(PAGE_NOTICE) && m_editNotice && Widget::ROI{m_textX, m_textY, m_textW, m_noticeH}.in(eventX - (m.x - m.ro->x), eventY - (m.y - m.ro->y))){
                    m_noticeEditBoard.setFocus(true);
                    return true;
                }
                return consumeFocus(true);
            }
        case SDL_EVENT_MOUSE_WHEEL:
            {
                if(!m.in(to_d(event.wheel.mouse_x), to_d(event.wheel.mouse_y))){
                    return false;
                }

                if(const auto maxScroll = scrollMax(); maxScroll > 0){
                    m_slider.addValue(-event.wheel.y * ((m_page == PAGE_MEMBER) ? 1.0f : 20.0f) / maxScroll, false);
                }
                return true;
            }
        default:
            {
                return false;
            }
    }
}

void GuildBoard::setGuildInfo(SDGuildInfo sdGI)
{
    m_guild = std::move(sdGI);

    if(!findMember(m_selectedDBID)){
        m_selectedDBID = 0;
    }

    if(m_editNotice && !isLeader()){
        stopEditNotice();
    }
    loadNotice();
}

void GuildBoard::addChatMessage(const std::string &name, const std::string &content)
{
    const bool atBottom = (m_chatBoard.h() <= m_chatH) || (m_slider.getValue() >= 1.0f);
    const auto offset = scrollOffset(m_chatBoard.h() - m_chatH);

    m_chatBoard.addParXML(m_chatBoard.parCount(), {}, xmlf::toParString("%s: %s", name.c_str(), content.c_str()).c_str());
    if(const auto parCount = m_chatBoard.parCount(); parCount > m_chatLineCount){
        m_chatBoard.removePar(0, parCount - m_chatLineCount);
    }

    if(m_page == PAGE_CHAT){
        if(atBottom){
            m_slider.setValue(1.0f, false);
        }
        else if(const auto maxOffset = m_chatBoard.h() - m_chatH; maxOffset > 0){
            m_slider.setValue(to_f(offset) / maxOffset, false);
        }
    }
}

bool GuildBoard::pageVisible(PageType page) const
{
    return m_guild.guildID && (m_page == page);
}

void GuildBoard::setPage(PageType page)
{
    if(m_page != page){
        m_page = page;
        m_chatInput.setFocus(false);
        m_noticeEditBoard.setFocus(false);
        m_slider.setValue((page == PAGE_CHAT) ? 1.0f : 0.0f, false);
    }
}

const SDGuildMember *GuildBoard::findMember(uint64_t dbid) const
{
    for(const auto &member: m_guild.memberList){
        if(member.dbid == dbid){
            return &member;
        }
    }
    return nullptr;
}

bool GuildBoard::isLeader() const
{
    const auto member = findMember(m_processRun->getMyHeroDBID());
    return member && (member->rank == GUILDRANK_LEADER);
}

bool GuildBoard::checkGuild() const
{
    if(m_guild.guildID){
        return true;
    }

    m_processRun->addCBLog(CBLOG_ERR, u8"你还没有加入行会");
    return false;
}

bool GuildBoard::checkLeader() const
{
    if(!checkGuild()){
        return false;
    }

    if(isLeader()){
        return true;
    }

    m_processRun->addCBLog(CBLOG_ERR, u8"只有掌门人才可以这样做");
    return false;
}

int GuildBoard::scrollOffset(int maxOffset) const
{
    return std::max<int>(0, to_dround(maxOffset * m_slider.getValue()));
}

int GuildBoard::scrollMax() const
{
    switch(m_page){
        case PAGE_NOTICE: return (m_editNotice ? m_noticeEditBoard.h() : m_noticeBoard.h()) - m_noticeH;
        case PAGE_MEMBER: return to_d(m_guild.memberList.size()) - m_memberRowCount;
        case PAGE_CHAT  : return m_chatBoard.h() - m_chatH;
        default         : return 0;
    }
}

std::optional<size_t> GuildBoard::memberIndex(int row) const
{
    if(row >= 0 && row < m_memberRowCount){
        if(const auto index = to_uz(scrollOffset(to_d(m_guild.memberList.size()) - m_memberRowCount) + row); index < m_guild.memberList.size()){
            return index;
        }
    }
    return std::nullopt;
}

void GuildBoard::loadNotice()
{
    if(m_guild.notice.empty()){
        m_noticeBoard.loadXML(to_cstr(u8"<layout><par color=\"grey\">暂无公告</par></layout>"));
    }
    else{
        m_noticeBoard.loadXML(noticeXML(m_guild.notice).c_str());
    }
}

void GuildBoard::editNotice()
{
    if(!checkLeader()){
        return;
    }

    if(!m_editNotice){
        setPage(PAGE_NOTICE);
        m_editNotice = true;

        // clear() resets the cursor to the first par, loadXML() alone keeps it and can leave it out of range
        m_noticeEditBoard.clear();
        m_noticeEditBoard.loadXML(noticeXML(m_guild.notice).c_str());

        m_slider.setValue(0.0f, false);
        m_noticeEditBoard.setFocus(true);
        return;
    }

    const auto notice = m_noticeEditBoard.getText();
    if(notice.size() >= SYS_GUILDNOTICESIZE){
        m_processRun->addCBLog(CBLOG_ERR, u8"公告内容过长");
        return;
    }

    if(notice != m_guild.notice){
        m_processRun->requestSetGuildNotice(notice);
    }
    stopEditNotice();
}

void GuildBoard::stopEditNotice()
{
    m_editNotice = false;
    m_noticeEditBoard.setFocus(false);

    if(m_page == PAGE_NOTICE){
        m_slider.setValue(0.0f, false);
    }
}

void GuildBoard::sendChat()
{
    const auto message = m_chatInput.getRawString();
    if(message.find_first_not_of(" \t") == std::string::npos){
        return;
    }

    if(!checkGuild()){
        return;
    }

    if(message.size() >= m_chatMessageSize){
        m_processRun->addCBLog(CBLOG_ERR, u8"聊天内容过长");
        return;
    }

    m_processRun->requestGuildChat(message);
    m_chatInput.clear();
}

void GuildBoard::inviteMember()
{
    if(!checkLeader()){
        return;
    }

    auto inputBoard = dynamic_cast<InputStringBoard *>(m_processRun->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    inputBoard->waitInput(
    {
        .layoutString = u8"<layout><par>请输入要邀请的角色名字</par></layout>",
        .ime = m_processRun->getRuntimeConfig<RTCFG_IME>(),
        .onAccept = [this](std::u8string input)
        {
            const std::string name(input.begin(), input.end());
            if(name.empty() || name.size() >= SYS_NAMESIZE){
                m_processRun->addCBLog(CBLOG_ERR, u8"无效的角色名字");
                return;
            }
            m_processRun->requestInviteGuildMember(name);
        },

        .onReject = []{},
        .validate = [](const std::string &currentInput, const std::string &newInput)
        {
            return currentInput.size() + newInput.size() < SYS_NAMESIZE;
        },
    });
}

void GuildBoard::removeMember()
{
    if(!checkLeader()){
        return;
    }

    const auto member = findMember(m_selectedDBID);
    if(!member){
        m_processRun->addCBLog(CBLOG_ERR, u8"请先选择要删除的成员");
        return;
    }

    if(member->dbid == m_processRun->getMyHeroDBID()){
        m_processRun->addCBLog(CBLOG_ERR, u8"不能删除自己");
        return;
    }

    auto inputBoard = dynamic_cast<InputStringBoard *>(m_processRun->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    inputBoard->waitChoice(to_u8rawstr("<layout>" + xmlf::toParString("确定要将%s逐出行会吗？", member->name.c_str()) + "</layout>"), {}, false, [dbid = member->dbid, this](size_t)
    {
        m_processRun->requestKickGuildMember(dbid);
    },

    []{});
}

void GuildBoard::leaveGuild()
{
    if(!checkGuild()){
        return;
    }

    auto inputBoard = dynamic_cast<InputStringBoard *>(m_processRun->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    if(isLeader()){
        inputBoard->waitChoice(u8"<layout><par>确定要解散行会吗？</par></layout>", {}, false, [this](size_t)
        {
            m_processRun->requestDisbandGuild();
        },

        []{});
    }
    else{
        inputBoard->waitChoice(u8"<layout><par>确定要退出行会吗？</par></layout>", {}, false, [this](size_t)
        {
            m_processRun->requestLeaveGuild();
        },

        []{});
    }
}

void GuildBoard::editMemberPosition()
{
    if(!checkLeader()){
        return;
    }

    const auto member = findMember(m_selectedDBID);
    if(!member){
        m_processRun->addCBLog(CBLOG_ERR, u8"请先选择要编辑职位的成员");
        return;
    }

    std::vector<std::u8string> choiceList
    {
        u8"<layout><par>修改职位名称</par></layout>",
    };

    if(member->dbid != m_processRun->getMyHeroDBID()){
        choiceList.push_back(u8"<layout><par>转让掌门人</par></layout>");
    }

    auto inputBoard = dynamic_cast<InputStringBoard *>(m_processRun->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    inputBoard->waitChoice(to_u8rawstr("<layout>" + xmlf::toParString("编辑%s的职位", member->name.c_str()) + "</layout>"), std::move(choiceList), false, [dbid = member->dbid, name = member->name, this](size_t choice)
    {
        // InputStringBoard hides itself after onAccept returns, open the next dialog on the next tick
        m_processRun->addDelay(0, [choice, dbid, name, this]
        {
            if(choice == 0){
                editMemberTitle(dbid);
            }
            else{
                transferLeader(dbid, name);
            }
        });
    },

    []{});
}

void GuildBoard::editMemberTitle(uint32_t dbid)
{
    auto inputBoard = dynamic_cast<InputStringBoard *>(m_processRun->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    inputBoard->waitInput(
    {
        .layoutString = u8"<layout><par>请输入新的职位名称</par></layout>",
        .ime = m_processRun->getRuntimeConfig<RTCFG_IME>(),
        .onAccept = [dbid, this](std::u8string input)
        {
            const std::string title(input.begin(), input.end());
            if(title.empty() || title.size() >= SYS_GUILDTITLESIZE){
                m_processRun->addCBLog(CBLOG_ERR, u8"无效的职位名称");
                return;
            }
            m_processRun->requestSetGuildMemberTitle(dbid, title);
        },

        .onReject = []{},
        .validate = [](const std::string &currentInput, const std::string &newInput)
        {
            return currentInput.size() + newInput.size() < SYS_GUILDTITLESIZE;
        },
    });
}

void GuildBoard::transferLeader(uint32_t dbid, const std::string &name)
{
    auto inputBoard = dynamic_cast<InputStringBoard *>(m_processRun->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    inputBoard->waitChoice(to_u8rawstr("<layout>" + xmlf::toParString("确定要将掌门人转让给%s吗？", name.c_str()) + "</layout>"), {}, false, [dbid, this](size_t)
    {
        m_processRun->requestTransferGuildLeader(dbid);
    },

    []{});
}

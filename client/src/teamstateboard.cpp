#include <type_traits>
#include "strf.hpp"
#include "uidf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "combatnode.hpp"
#include "processrun.hpp"
#include "pngtexoffdb.hpp"
#include "soundeffectdb.hpp"
#include "inventoryboard.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_equipDB;
extern SoundEffectDB *g_seffDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

TeamStateBoard::TeamStateBoard(int argX, int argY, ProcessRun *runPtr, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .x = argX,
          .y = argY,
          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_enableTeam
      {{
          .x = 24,
          .y = 47,

          .texIDList
          {
              .on   = 0X00000200,
              .down = 0X00000201,
          },

          .parent{this},
      }}

    , m_switchShow
      {{
          .x = 19,
          .y = 0, // reset by adjustButtonPos()

          .texIDList
          {
              .on   = 0X00000160,
              .down = 0X00000161,
          },

          .onTrigger = [this](Widget *, int)
          {
              m_showCandidateList = !m_showCandidateList;
              if(m_showCandidateList){
                  m_addMember.setActive(true);
                  m_deleteMember.setActive(false);
              }
              else{
                  m_addMember.setActive(false);
                  m_deleteMember.setActive(true);
              }
          },

          .parent{this},
      }}

    , m_addMember
      {{
          .x = 72,
          .y = 0, // reset by adjustButtonPos()

          .texIDList
          {
              .off  = 0X00000170,
              .on   = 0X00000170,
              .down = 0X00000171,
          },

          .onTrigger = [this](Widget *, int)
          {
              if(m_showCandidateList && (m_selectedIndex[m_showCandidateList] >= 0)){
                  m_processRun->requestJoinTeam(getSDTeamPlayer(m_selectedIndex[m_showCandidateList]).uid);
              }
          },

          .parent{this},
      }}

    , m_deleteMember
      {{
          .x = 125,
          .y = 0, // reset by adjustButtonPos()

          .texIDList
          {
              .off  = 0X00000180,
              .on   = 0X00000180,
              .down = 0X00000181,
          },

          .onTrigger = [this](Widget *, int)
          {
              if(!m_showCandidateList && (m_selectedIndex[m_showCandidateList] >= 0)){
                  m_processRun->requestLeaveTeam(getSDTeamPlayer(m_selectedIndex[m_showCandidateList]).uid);
              }
          },

          .parent{this},
      }}

    , m_refresh
      {{
          .x = 177,
          .y = 0, // reset by adjustButtonPos()

          .texIDList
          {
              .on   = 0X00000190,
              .down = 0X00000191,
          },

          .onTrigger = [this](Widget *, int)
          {
              refresh();
          },

          .parent{this},
      }}

    , m_close
      {{
          .x = 217,
          .y = 0, // reset by adjustButtonPos()

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

    , m_processRun(runPtr)
{
    setShow(false);
    const auto [texW, texH] = []() -> std::tuple<int, int>
    {
        if(auto texPtr = g_progUseDB->retrieve(0X00000150)){
            return SDLDeviceHelper::getTextureSize(texPtr);
        }
        else{
            return {258, 244};
        }
    }();

    setW(texW);
    setH(texH - m_uidRegionH + to_d(lineShowCount()) * lineHeight());

    adjustButtonPos();

    if(m_showCandidateList){
        m_addMember.setActive(true);
        m_deleteMember.setActive(false);
    }
    else{
        m_addMember.setActive(false);
        m_deleteMember.setActive(true);
    }
}

void TeamStateBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    const auto remapXDiff = m.x - m.ro->x;
    const auto remapYDiff = m.y - m.ro->y;

    if(auto texPtr = g_progUseDB->retrieve(0X00000150)){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        const auto texRepeatStartY = m_uidRegionY + (m_uidRegionH - m_texRepeatH) / 2;

        g_sdlDevice->drawTexture(texPtr, remapXDiff, remapYDiff, 0, 0, texW, texRepeatStartY);

        const auto neededUIDRegionH = lineShowCount() * lineHeight();
        const auto neededRepeatTexH = neededUIDRegionH - (m_uidRegionH - m_texRepeatH);

        const auto repeatCount = neededRepeatTexH / m_texRepeatH;
        const auto repeatRest  = neededRepeatTexH % m_texRepeatH;

        for(size_t i = 0; i < repeatCount; ++i){
            g_sdlDevice->drawTexture(texPtr, remapXDiff, remapYDiff + texRepeatStartY + i * m_texRepeatH, 0, texRepeatStartY, texW, m_texRepeatH);
        }

        if(repeatRest > 0){
            g_sdlDevice->drawTexture(texPtr, remapXDiff, remapYDiff + texRepeatStartY + repeatCount * m_texRepeatH, 0, texRepeatStartY, texW, repeatRest);
        }

        const int texRepeatEndY = texRepeatStartY + m_texRepeatH;
        g_sdlDevice->drawTexture(texPtr, remapXDiff, remapYDiff + texRepeatStartY + neededRepeatTexH, 0, texRepeatEndY, texW, texH - texRepeatEndY);

        LabelBoard header{{.label = m_showCandidateList ? u8"申请加入" : u8"当前队伍", .font{.color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF)}}};
        header.draw({.dir=DIR_NONE, .x{remapXDiff + w() / 2}, .y{remapYDiff + 57}});
    }

    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();

    std::string nameText;
    XMLTypeset line(-1, LALIGN_LEFT, false, m_font, m_fontSize, m_fontStyle, m_fontColor);

    for(size_t i = 0; (i < lineShowCount()) && (i + m_startIndex[m_showCandidateList] < lineCount()); ++i){
        if((m_selectedIndex[m_showCandidateList] >= 0) && (to_d(i) + m_startIndex[m_showCandidateList] == m_selectedIndex[m_showCandidateList])){
            g_sdlDevice->fillRectangle(m_selectedBGColor, remapXDiff + m_uidRegionX, remapYDiff + m_uidRegionY + i * lineHeight(), m_uidRegionW, lineHeight());
        }

        if(mathf::pointInRectangle<int>(mousePX, mousePY, remapXDiff + m_uidRegionX, remapYDiff + m_uidRegionY + i * lineHeight(), m_uidRegionW, lineHeight())){
            g_sdlDevice->fillRectangle(m_hoveredColor, remapXDiff + m_uidRegionX, remapYDiff + m_uidRegionY + i * lineHeight(), m_uidRegionW, lineHeight());
        }

        const auto &sdTP = getSDTeamPlayer(i + m_startIndex[m_showCandidateList]);
        nameText = sdTP.name;

        // make every uid a line
        // need line number to select uid

        if(nameText.empty()){
            nameText = uidf::getUIDString(sdTP.uid);
        }

        line.clear();
        line.loadXML(str_printf("<par>%d %s</par>", to_d(i) + m_startIndex[m_showCandidateList], nameText.c_str()).c_str());
        line.draw(
        {
            .x=remapXDiff + m_uidRegionX + (m_uidRegionW - m_uidTextRegionW) / 2,
            .y=remapYDiff + m_uidRegionY + m_lineSpace / 2 + static_cast<int>(i) * lineHeight(),
            .ro
            {
                0,
                0,
                std::min<int>(line.pw(), m_uidTextRegionW),
                line.ph(),
            }
        });
    }

    drawChild(&m_enableTeam,   m);
    drawChild(&m_switchShow,   m);
    drawChild(&m_addMember,    m);
    drawChild(&m_deleteMember, m);
    drawChild(&m_refresh,      m);
    drawChild(&m_close,        m);
}

bool TeamStateBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_enableTeam  .processEventParent(event, valid, m)){ return true; }
    if(m_switchShow  .processEventParent(event, valid, m)){ return true; }
    if(m_addMember   .processEventParent(event, valid, m)){ return true; }
    if(m_deleteMember.processEventParent(event, valid, m)){ return true; }
    if(m_refresh     .processEventParent(event, valid, m)){ return true; }
    if(m_close       .processEventParent(event, valid, m)){ return true; }

    const auto remapXDiff = m.x - m.ro->x;
    const auto remapYDiff = m.y - m.ro->y;

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                int selectedLine = -1;
                for(size_t i = 0; i < lineShowCount(); ++i){
                    const auto lineX = remapXDiff + m_uidRegionX;
                    const auto lineY = remapYDiff + m_uidRegionY + i * lineHeight();
                    if(mathf::pointInRectangle<int>(event.button.x, event.button.y, lineX, lineY, m_uidRegionW, lineHeight())){
                        selectedLine = i;
                        break;
                    }
                }

                if(selectedLine >= 0){
                    if(selectedLine + m_startIndex[m_showCandidateList] < to_d(lineCount())){
                        m_selectedIndex[m_showCandidateList] = selectedLine + m_startIndex[m_showCandidateList];
                        if(event.button.clicks >= 2){
                            // can show hero state board here
                        }
                    }
                    else{
                        // clicked in uidRegion, but on invalid line
                        // reset selection
                        m_selectedIndex[m_showCandidateList] = -1;
                    }
                }
                else{
                    // clicked outside uidRegion
                    // ignore event
                }

                return consumeFocus(true);
            }
        case SDL_MOUSEWHEEL:
            {
                const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
                if(mathf::pointInRectangle<int>(mousePX, mousePY, remapXDiff + m_uidRegionX, remapYDiff + m_uidRegionY, m_uidRegionW, lineHeight() * lineShowCount())){
                    if(lineCount() <= lineShowCount()){
                        m_startIndex[m_showCandidateList] = 0;
                    }
                    else if(event.wheel.y < 0){
                        m_startIndex[m_showCandidateList] = std::min<int>(m_startIndex[m_showCandidateList] + 1, lineCount() - lineShowCount());
                    }
                    else{
                        m_startIndex[m_showCandidateList] = std::max<int>(m_startIndex[m_showCandidateList] - 1, 0);
                    }
                }
                return consumeFocus(true);
            }
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || focus())){
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
        default:
            {
                return consumeFocus(false);
            }
    }
}

void TeamStateBoard::refresh()
{
    adjustButtonPos();
}

void TeamStateBoard::adjustButtonPos()
{
    const int buttonY = m_uidRegionY + lineShowCount() * lineHeight() + 14;

    m_switchShow  .moveYTo(buttonY);
    m_addMember   .moveYTo(buttonY);
    m_deleteMember.moveYTo(buttonY);
    m_refresh     .moveYTo(buttonY);
    m_close       .moveYTo(buttonY + 8);
}

void TeamStateBoard::addTeamCandidate(SDTeamCandidate sdTC)
{
    if(std::find_if(m_teamMemberList.memberList.begin(), m_teamMemberList.memberList.end(), [&sdTC](const auto &player)
    {
        return player.uid == sdTC.player.uid;
    }) != m_teamMemberList.memberList.end()){
        return;
    }

    for(auto p = m_teamCandidateList.begin(); p != m_teamCandidateList.end();){
        if(p->first.player.uid == sdTC.player.uid){
            p = m_teamCandidateList.erase(p);
        }
        else{
            ++p;
        }
    }

    m_teamCandidateList.push_front(std::make_pair(std::move(sdTC), hres_timer()));
    if(!show()){

    }
}

void TeamStateBoard::setTeamMemberList(SDTeamMemberList sdTML)
{
    m_teamMemberList = std::move(sdTML);
    const auto fnIsTeamMember = [this](uint64_t uid)
    {
        return std::find_if(m_teamMemberList.memberList.begin(), m_teamMemberList.memberList.end(), [uid](const auto &member) -> bool
        {
            return member.uid == uid;
        }) != m_teamMemberList.memberList.end();
    };

    for(auto p = m_teamCandidateList.begin(); p != m_teamCandidateList.end();){
        if(fnIsTeamMember(p->first.player.uid)){
            p = m_teamCandidateList.erase(p);
        }
        else{
            p++;
        }
    }

    if(m_selectedIndex[false] >= to_d(m_teamMemberList.memberList.size())){
        m_selectedIndex[false] = -1;
    }
}

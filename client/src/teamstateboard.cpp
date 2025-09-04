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

TeamStateBoard::TeamStateBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
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

    , m_enableTeam
      {
          DIR_UPLEFT,
          24,
          47,
          {SYS_U32NIL, 0X00000200, 0X00000201},
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

    , m_switchShow
      {
          DIR_UPLEFT,
          19,
          0, // reset by adjustButtonPos()
          {SYS_U32NIL, 0X00000160, 0X00000161},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
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

    , m_addMember
      {
          DIR_UPLEFT,
          72,
          0, // reset by adjustButtonPos()
          {0X00000170, 0X00000170, 0X00000171},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(m_showCandidateList && (m_selectedIndex[m_showCandidateList] >= 0)){
                  m_processRun->requestJoinTeam(getSDTeamPlayer(m_selectedIndex[m_showCandidateList]).uid);
              }
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

    , m_deleteMember
      {
          DIR_UPLEFT,
          125,
          0, // reset by adjustButtonPos()
          {0X00000180, 0X00000180, 0X00000181},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(!m_showCandidateList && (m_selectedIndex[m_showCandidateList] >= 0)){
                  m_processRun->requestLeaveTeam(getSDTeamPlayer(m_selectedIndex[m_showCandidateList]).uid);
              }
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

    , m_refresh
      {
          DIR_UPLEFT,
          177,
          0, // reset by adjustButtonPos()
          {SYS_U32NIL, 0X00000190, 0X00000191},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              refresh();
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

    , m_close
      {
          DIR_UPLEFT,
          217,
          0, // reset by adjustButtonPos()
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
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

void TeamStateBoard::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    const auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    const auto remapXDiff = dstX - roiOpt->x;
    const auto remapYDiff = dstY - roiOpt->y;

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

        LabelBoard header(DIR_NONE, 0, 0, m_showCandidateList ? u8"申请加入" : u8"当前队伍", 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF));
        header.drawAt(DIR_NONE, remapXDiff + w() / 2, remapYDiff + 57);
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
        line.drawEx(remapXDiff + m_uidRegionX + (m_uidRegionW - m_uidTextRegionW) / 2, remapYDiff + m_uidRegionY + m_lineSpace / 2 + i * lineHeight(), 0, 0, std::min<int>(line.pw(), m_uidTextRegionW), line.ph());
    }

    drawChildEx(&m_enableTeam, dstX, dstY, roiOpt.value());
    drawChildEx(&m_switchShow, dstX, dstY, roiOpt.value());
    drawChildEx(&m_addMember, dstX, dstY, roiOpt.value());
    drawChildEx(&m_deleteMember, dstX, dstY, roiOpt.value());
    drawChildEx(&m_refresh, dstX, dstY, roiOpt.value());
    drawChildEx(&m_close, dstX, dstY, roiOpt.value());
}

bool TeamStateBoard::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_enableTeam.processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){
        return true;
    }

    if(m_switchShow.processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){
        return true;
    }

    if(m_addMember.processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){
        return true;
    }

    if(m_deleteMember.processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){
        return true;
    }

    if(m_refresh.processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){
        return true;
    }

    if(m_close.processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){
        return true;
    }

    const auto remapXDiff = startDstX - roiOpt->x;
    const auto remapYDiff = startDstY - roiOpt->y;

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
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y, startDstX, startDstY, roiOpt.value()) || focus())){
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

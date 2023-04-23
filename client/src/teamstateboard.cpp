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
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }

    , m_createTeam
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
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }

    , m_addMember
      {
          DIR_UPLEFT,
          72,
          0, // reset by adjustButtonPos()
          {SYS_U32NIL, 0X00000170, 0X00000171},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }

    , m_deleteMember
      {
          DIR_UPLEFT,
          125,
          0, // reset by adjustButtonPos()
          {SYS_U32NIL, 0X00000180, 0X00000181},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
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
          [this]()
          {
              refresh();
          },

          0,
          0,
          0,
          0,

          true,
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
          [this]()
          {
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
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

    m_w = texW;
    m_h = texH - m_uidRegionH + lineCount() * lineHeight();

    adjustButtonPos();
}

void TeamStateBoard::drawEx(int, int, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X00000150)){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        const auto texRepeatStartY = m_uidRegionY + (m_uidRegionH - m_texRepeatH) / 2;

        g_sdlDevice->drawTexture(texPtr, x(), y(), 0, 0, texW, texRepeatStartY);

        const auto neededUIDRegionH = lineCount() * lineHeight();
        const auto neededRepeatTexH = neededUIDRegionH - (m_uidRegionH - m_texRepeatH);

        const auto repeatCount = neededRepeatTexH / m_texRepeatH;
        const auto repeatRest  = neededRepeatTexH % m_texRepeatH;

        for(int i = 0; i < repeatCount; ++i){
            g_sdlDevice->drawTexture(texPtr, x(), y() + texRepeatStartY + i * m_texRepeatH, 0, texRepeatStartY, texW, m_texRepeatH);
        }

        if(repeatRest > 0){
            g_sdlDevice->drawTexture(texPtr, x(), y() + texRepeatStartY + repeatCount * m_texRepeatH, 0, texRepeatStartY, texW, repeatRest);
        }

        const int texRepeatEndY = texRepeatStartY + m_texRepeatH;
        g_sdlDevice->drawTexture(texPtr, x(), y() + texRepeatStartY + neededRepeatTexH, 0, texRepeatEndY, texW, texH - texRepeatEndY);
    }

    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();

    std::string nameText;
    XMLTypeset line(-1, LALIGN_LEFT, false, m_font, m_fontSize, m_fontStyle, m_fontColor);

    for(int i = 0; (i < lineCount()) && (i + m_startIndex < to_d(m_uidList.size())); ++i){
        if((m_selectedIndex >= 0) && (i + m_startIndex == m_selectedIndex)){
            g_sdlDevice->fillRectangle(m_selectedBGColor, x() + m_uidRegionX, y() + m_uidRegionY + i * lineHeight(), m_uidRegionW, lineHeight());
        }

        if(mathf::pointInRectangle<int>(mousePX, mousePY, x() + m_uidRegionX, y() + m_uidRegionY + i * lineHeight(), m_uidRegionW, lineHeight())){
            g_sdlDevice->fillRectangle(m_hoveredColor, x() + m_uidRegionX, y() + m_uidRegionY + i * lineHeight(), m_uidRegionW, lineHeight());
        }

        const auto uid = m_uidList.at(i + m_startIndex);

        nameText.clear();
        if(uidf::isPlayer(uid)){
            if(const auto heroPtr = dynamic_cast<Hero *>(m_processRun->findUID(uid, true))){
                nameText = heroPtr->getName();
            }
        }

        // make every uid a line
        // need line number to select uid

        if(nameText.empty()){
            nameText = uidf::getUIDString(uid);
        }

        line.clear();
        line.loadXML(str_printf("<par>%s</par>", nameText.c_str()).c_str());
        line.drawEx(x() + m_uidRegionX + (m_uidRegionW - m_uidTextRegionW) / 2, y() + m_uidRegionY + m_lineSpace / 2 + i * lineHeight(), 0, 0, std::min<int>(line.pw(), m_uidTextRegionW), line.ph());
    }

    m_enableTeam  .draw();
    m_createTeam  .draw();
    m_addMember   .draw();
    m_deleteMember.draw();
    m_refresh     .draw();
    m_close       .draw();
}

bool TeamStateBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_enableTeam.processEvent(event, valid)){
        return true;
    }

    if(m_createTeam.processEvent(event, valid)){
        return true;
    }

    if(m_addMember.processEvent(event, valid)){
        return true;
    }

    if(m_deleteMember.processEvent(event, valid)){
        return true;
    }

    if(m_refresh.processEvent(event, valid)){
        return true;
    }

    if(m_close.processEvent(event, valid)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                for(int i = 0; i < lineCount(); ++i){
                    const auto lineX = x() + m_uidRegionX;
                    const auto lineY = y() + m_uidRegionY + i * lineHeight();

                    if(mathf::pointInRectangle<int>(event.button.x - lineX, event.button.y - lineY, 0, 0, m_uidRegionW, lineHeight())){
                        m_selectedIndex = i + m_startIndex;
                        if(event.button.clicks >= 2){
                            // can show hero state board here
                        }
                    }
                }
                return consumeFocus(true);
            }
        case SDL_MOUSEWHEEL:
            {
                const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
                if(mathf::pointInRectangle<int>(mousePX, mousePY, x() + m_uidRegionX, y() + m_uidRegionY, m_uidRegionW, lineHeight() * lineCount())){
                    if(event.wheel.y < 0){
                        m_startIndex = std::min<int>(m_startIndex + 1, m_uidList.size() - lineCount());
                    }
                    else{
                        m_startIndex = std::max<int>(m_startIndex - 1, 0);
                    }
                }
                return consumeFocus(true);
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
        default:
            {
                return consumeFocus(false);
            }
    }
}

void TeamStateBoard::refresh()
{
    const auto selectedUID = (m_selectedIndex >= 0) ? m_uidList.at(m_selectedIndex) : 0;

    m_startIndex = 0;
    m_uidList.clear();

    for(const auto &[uid, coPtr]: m_processRun->getCOList()){
        if(uid == m_processRun->getMyHeroUID()){
            continue;
        }

        if(uidf::isPlayer(uid)){
            m_uidList.push_back(uid);
        }
    }

    std::sort(m_uidList.begin(), m_uidList.end());

    m_selectedIndex = -1;
    if(selectedUID){
        for(size_t i = 0; i < m_uidList.size(); ++i){
            if(m_uidList[i] == selectedUID){
                m_selectedIndex = to_d(i);
                break;
            }
        }
    }

    adjustButtonPos();
}

void TeamStateBoard::adjustButtonPos()
{
    const auto buttonY = m_uidRegionY + lineCount() * lineHeight() + 14;

    m_createTeam  .moveTo({}, buttonY);
    m_addMember   .moveTo({}, buttonY);
    m_deleteMember.moveTo({}, buttonY);
    m_refresh     .moveTo({}, buttonY);
    m_close       .moveTo({}, buttonY + 8);
}

/*
 * =====================================================================================
 *
 *       Filename: skillboard.cpp
 *        Created: 10/08/2017 19:22:30
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "dbcomid.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "skillboard.hpp"
#include "dbcomrecord.hpp"
#include "tritexbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

SkillBoard::MagicIconButton::MagicIconButton(int argX, int argY, SkillBoard::MagicIconData *iconDataPtr, Widget *widgetPtr, bool autoDelete)
    : WidgetGroup
      {
          DIR_UPLEFT,
          argX,
          argY,
          0,
          0,
          widgetPtr,
          autoDelete,
      }

    , m_magicIconDataPtr([iconDataPtr]()
      {
          fflassert(iconDataPtr);
          return iconDataPtr;
      }())

    , m_icon
      {
          DIR_UPLEFT,
          0,
          0,

          {
              DBCOM_MAGICRECORD(m_magicIconDataPtr->magicID).icon,
              DBCOM_MAGICRECORD(m_magicIconDataPtr->magicID).icon,
              DBCOM_MAGICRECORD(m_magicIconDataPtr->magicID).icon,
          },

          nullptr,
          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          false,
          true,
          this,
          false,
      }
{
    // leave some pixels to draw level label
    // since level can change during run, can't get the exact size here
    m_w = m_icon.w() + 8;
    m_h = m_icon.h() + 8;
}

void SkillBoard::MagicIconButton::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(m_magicIconDataPtr->level > 0){
        WidgetGroup::drawEx(dstX, dstY, srcX, srcY, srcW, srcH);

        const LabelBoard magicLevel
        {
            DIR_UPLEFT,
            0,
            0,
            str_printf(u8"%d", m_magicIconDataPtr->level).c_str(),

            3,
            12,
            0,

            colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
        };
        magicLevel.drawAt(DIR_UPLEFT, (dstX - srcX) + (m_icon.w() - 2), (dstY - srcY) + (m_icon.h() - 1), dstX, dstY, srcW, srcH);

        if(m_magicIconDataPtr->key != '\0'){
            const LabelShadowBoard magicKey
            {
                DIR_UPLEFT,
                0,
                0,
                2,
                2,
                str_printf(u8"%c", std::toupper(m_magicIconDataPtr->key)).c_str(),

                3,
                20,
                0,

                colorf::RGBA(0XFF, 0X80, 0X00, 0XE0),
                colorf::RGBA(0X00, 0X00, 0X00, 0XE0),
            };
            magicKey.drawAt(DIR_UPLEFT, (dstX - srcX + 2), (dstY - srcY + 2), dstX, dstY, srcW, srcH);
        }
    }
}

SkillBoard::SkillPage::SkillPage(uint32_t pageImage, Widget *widgetPtr, bool autoDelete)
    : WidgetGroup
      {
          DIR_UPLEFT,
          SkillBoard::getPageRectange().at(0),
          SkillBoard::getPageRectange().at(1),
          0, // reset in ctor body
          0,
          widgetPtr,
          autoDelete,
      }
    , m_pageImage(pageImage)
{
    std::tie(m_w, m_h) = [this]() -> std::tuple<int, int>
    {
        if(auto texPtr = g_progUseDB->Retrieve(m_pageImage)){
            return SDLDeviceHelper::getTextureSize(texPtr);
        }

        const auto r = SkillBoard::getPageRectange();
        return {r[2], r[3]};
    }();
}

void SkillBoard::SkillPage::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(auto texPtr = g_progUseDB->Retrieve(m_pageImage)){
        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        if(mathf::ROICrop(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    texW,
                    texH)){
            g_sdlDevice->drawTexture(texPtr, dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
        }
    }
    WidgetGroup::drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}

SkillBoard::SkillBoard(int nX, int nY, ProcessRun *runPtr, Widget *pwidget, bool autoDelete)
    : Widget(DIR_UPLEFT, nX, nY, 0, 0, pwidget, autoDelete)
    , m_magicIconDataList
      {
          // not only for currnet user
          // list all possible magics for all types of user
          {DBCOM_MAGICID(u8"雷电术"  ),  12,  78},
          {DBCOM_MAGICID(u8"魔法盾"  ), 252, 143},
          {DBCOM_MAGICID(u8"召唤骷髅"),  12,  13},
          {DBCOM_MAGICID(u8"召唤神兽"),  12,  78},
          {DBCOM_MAGICID(u8"灵魂火符"), 192,  13},
          {DBCOM_MAGICID(u8"冰月神掌"),  12,  13},
          {DBCOM_MAGICID(u8"冰月震天"),  12,  78},
          {DBCOM_MAGICID(u8"冰沙掌"  ), 132, 143},
          {DBCOM_MAGICID(u8"地狱火"  ), 132,  78},
          {DBCOM_MAGICID(u8"火球术"  ),  12,  13},
          {DBCOM_MAGICID(u8"大火球"  ),  12,  78},
          {DBCOM_MAGICID(u8"焰天火雨"),  12, 208},
          {DBCOM_MAGICID(u8"火墙"    ), 132, 143},
          {DBCOM_MAGICID(u8"疾光电影"), 132, 143},
      }

    , m_skillPageList([this]() -> std::vector<SkillBoard::SkillPage *>
      {
          std::vector<SkillBoard::SkillPage *> pageList;
          pageList.reserve(8);

          for(int i = 0; i < 8; ++i){
              auto pagePtr = new SkillBoard::SkillPage
              {
                  to_u32(0X05000010 + to_u32(i)),
                  this,
                  true,
              };

              for(auto &iconRef: m_magicIconDataList){
                  if(i == getSkillPageIndex(iconRef.magicID)){
                      pagePtr->addIcon(&iconRef);
                  }
              }
              pageList.push_back(pagePtr);
          }
          return pageList;
      }())

    , m_tabButtonList([this]() -> std::vector<TritexButton *>
      {
          std::vector<TritexButton *> tabButtonList;
          tabButtonList.reserve(8);

          const int tabX = 45;
          const int tabY = 10;
          const int tabW = 34;

          // don't know WTF
          // can't compile if use std::vector<TritexButton>
          // looks for complicated parameter list gcc can't forward correctly ???

          for(int i = 0; i < 8; ++i){
              tabButtonList.push_back(new TritexButton
              {
                  DIR_UPLEFT,
                  tabX + tabW * i,
                  tabY,

                  {
                      SYS_TEXNIL,
                      0X05000020 + to_u32(i),
                      0X05000030 + to_u32(i),
                  },

                  [i, this]()
                  {
                      m_cursorOnTabIndex = i;
                  },

                  [i, this]()
                  {
                      if(i != m_cursorOnTabIndex){
                          return;
                      }
                      m_cursorOnTabIndex = -1;
                  },

                  [i, this]()
                  {
                      if(m_selectedTabIndex == i){
                          return;
                      }

                      m_tabButtonList.at(i)->setTexID(
                      {
                          0X05000030 + to_u32(i),
                          0X05000030 + to_u32(i),
                          0X05000030 + to_u32(i),
                      });

                      m_tabButtonList.at(m_selectedTabIndex)->setOff();
                      m_tabButtonList.at(m_selectedTabIndex)->setTexID(
                      {
                          SYS_TEXNIL,
                          0X05000020 + to_u32(m_selectedTabIndex),
                          0X05000030 + to_u32(m_selectedTabIndex),
                      });

                      m_selectedTabIndex = i;
                      m_slider.setValue(0);

                      const auto r = getPageRectange();
                      for(auto pagePtr: m_skillPageList){
                          pagePtr->moveTo(r[0], r[1]);
                      }
                  },

                  0,
                  0,
                  0,
                  0,

                  false,
                  false,
                  this,
                  true,
              });

              if(i == m_selectedTabIndex){
                  m_tabButtonList.at(i)->setTexID(
                  {
                      0X05000030 + to_u32(i),
                      0X05000030 + to_u32(i),
                      0X05000030 + to_u32(i),
                  });
              }
          }
          return tabButtonList;
      }())

    , m_slider
      {
          DIR_UPLEFT,
          326,
          74,

          266,
          2,

          [this](float value)
          {
              const auto r = SkillBoard::getPageRectange();
              auto pagePtr = m_skillPageList.at(m_selectedTabIndex);

              if(r[3] < pagePtr->h()){
                  pagePtr->moveTo(r[0], r[1] - (pagePtr->h() - r[3]) * value);
              }
          },
          this,
      }

    , m_closeButton
      {
          DIR_UPLEFT,
          317,
          402,
          {SYS_TEXNIL, 0X0000001C, 0X0000001D},

          nullptr,
          nullptr,
          [this]()
          {
              show(false);
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }
    , m_processRun(runPtr)
{
    show(false);
    if(auto texPtr = g_progUseDB->Retrieve(0X05000000)){
        std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid inventory frame texture");
    }
}

void SkillBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->Retrieve(0X05000000)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }

    drawTabName();
    m_slider.draw();
    m_closeButton.draw();

    for(auto buttonPtr: m_tabButtonList){
        buttonPtr->draw();
    }

    const auto r = SkillBoard::getPageRectange();
    auto pagePtr = m_skillPageList.at(m_selectedTabIndex);
    pagePtr->drawEx(dstX + r[0], dstY + r[1], r[0] - pagePtr->dx(), r[1] - pagePtr->dy(), r[2], r[3]);
}

bool SkillBoard::MagicIconButton::processEvent(const SDL_Event &event, bool valid)
{
    const auto result = m_icon.processEvent(event, valid);
    if(event.type == SDL_KEYDOWN && cursorOn()){
        if(const auto key = SDLDeviceHelper::getKeyChar(event, false); (key >= '0' && key <= '9') || (key >= 'a' && key <= 'z')){
            dynamic_cast<SkillPage *>(m_parent)->setMagicKey(m_magicIconDataPtr->magicID, key);
            return focusConsume(this, true);
        }
    }
    return result;
}

bool SkillBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return focusConsume(this, false);
    }

    if(!show()){
        return focusConsume(this, false);
    }

    if(m_closeButton.processEvent(event, valid)){
        return focusConsume(this, show());
    }

    bool tabConsumed = false;
    for(auto buttonPtr: m_tabButtonList){
        tabConsumed |= buttonPtr->processEvent(event, valid && !tabConsumed);
    }

    if(tabConsumed){
        return focusConsume(this, true);
    }

    if(m_slider.processEvent(event, valid)){
        return focusConsume(this, true);
    }

    const auto r = getPageRectange();
    const auto loc = SDLDeviceHelper::getMousePLoc();
    const bool captureEvent = loc && mathf::pointInRectangle(loc.x, loc.y, x() + r[0], y() + r[1], r[2], r[3]);

    if(m_skillPageList.at(m_selectedTabIndex)->processEvent(event, captureEvent && valid)){
        return focusConsume(this, true);
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y) || focus())){
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, x() + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, y() + event.motion.yrel));

                    moveBy(newX - x(), newY - y());
                    return focusConsume(this, true);
                }
                return focusConsume(this, false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            return focusConsume(this, in(event.button.x, event.button.y));
                        }
                    default:
                        {
                            return focusConsume(this, false);
                        }
                }
            }
        default:
            {
                return focusConsume(this, false);
            }
    }
}

std::tuple<uint32_t, int> SkillBoard::key2MagicID(char key) const
{
    for(const auto &iconCRef: m_magicIconDataList){
        if(std::tolower(iconCRef.key) == std::tolower(key)){
            return {iconCRef.magicID, iconCRef.angle()};
        }
    }
    return {0, 0};
}

void SkillBoard::drawTabName() const
{
    const LabelBoard tabName
    {
        DIR_UPLEFT,
        0,
        0,

        to_u8cstr([this]() -> std::u8string
        {
            if(m_cursorOnTabIndex >= 0){
                return str_printf(u8"元素【%s】", to_cstr(magicElemName(cursorOnElem())));
            }

            if(m_selectedTabIndex >= 0){
                for(const auto magicIconPtr: m_skillPageList.at(m_selectedTabIndex)->getMagicIconButtonList()){
                    if(magicIconPtr->cursorOn()){
                        if(const auto &mr = DBCOM_MAGICRECORD(magicIconPtr->getMagicIconDataPtr()->magicID)){
                            return str_printf(u8"元素【%s】%s", to_cstr(mr.element), to_cstr(mr.name));
                        }
                        else{
                            return str_printf(u8"元素【无】");
                        }
                    }
                }
                return str_printf(u8"元素【%s】", to_cstr(magicElemName(selectedElem())));
            }

            // fallback
            // shouldn't reach here
            return str_printf(u8"元素【无】");
        }()),

        1,
        12,
        0,

        colorf::WHITE + 255,
    };
    tabName.drawAt(DIR_UPLEFT, x() + 30, y() + 400);
}

void SkillBoard::setMagicLevel(uint32_t magicID, int level)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert(level >= 1 && level <= 3); // no way to delete a learnt magic

    for(auto &data: m_magicIconDataList){
        if(data.magicID == magicID){
            data.level = level;
            return;
        }
    }
    throw fflerror("no magic icon for magic: %s", to_cstr(DBCOM_MAGICRECORD(magicID).name));
}

void SkillBoard::setMagicKey(uint32_t magicID, char key)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9') || (key == '\0'));

    bool found = false;
    for(auto &data: m_magicIconDataList){
        if(data.magicID == magicID){
            found = true;
            data.key = key;
            break;
        }
    }

    if(!found){
        throw fflerror("no magic icon for magic: %s", to_cstr(DBCOM_MAGICRECORD(magicID).name));
    }

    for(auto &data: m_magicIconDataList){
        if(data.magicID != magicID && data.key == key){
            data.key = '\0';
        }
    }
    m_processRun->requestSetMagicKey(magicID, key);
}

void SkillBoard::setMagicCastTime(uint32_t magicID)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    for(auto &data: m_magicIconDataList){
        if(data.magicID == magicID){
            data.lastCastTime.reset();
            return;
        }
    }
    throw fflerror("no magic icon for magic: %s", to_cstr(DBCOM_MAGICRECORD(magicID).name));
}

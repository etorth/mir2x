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
#include "tritexbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

std::optional<char> SkillBoard::SkillBoardConfig::getMagicKey(uint32_t magicID) const
{
    if(auto p = m_learnedMagicList.find(magicID); p != m_learnedMagicList.end()){
        return p->second.key;
    }
    return {};
}

std::optional<int> SkillBoard::SkillBoardConfig::getMagicLevel(uint32_t magicID) const
{
    if(auto p = m_learnedMagicList.find(magicID); p != m_learnedMagicList.end()){
        return p->second.level;
    }
    return {};
}

void SkillBoard::SkillBoardConfig::setMagicLevel(uint32_t magicID, int level)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert(SkillBoard::getMagicIconGfx(magicID));

    fflassert(level >= 1);
    fflassert(level <= 3);

    if(auto p = m_learnedMagicList.find(magicID); p != m_learnedMagicList.end()){
        fflassert(level >= p->second.level);
        p->second.level = level;
    }
    else{
        m_learnedMagicList[magicID].level = level;
    }
}

void SkillBoard::SkillBoardConfig::setMagicKey(uint32_t magicID, std::optional<char> key)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert(SkillBoard::getMagicIconGfx(magicID));

    fflassert(hasMagicID(magicID));
    fflassert(!SkillBoard::getMagicIconGfx(magicID).passive);
    fflassert(!key.has_value() || (key.value() >= 'a' && key.value() <= 'z') || (key.value() >= '0' && key.value() <= '9'));

    m_learnedMagicList[magicID].key = key;
    if(key.has_value()){
        for(auto &p: m_learnedMagicList){
            if((p.first != magicID) && p.second.key == key){
                p.second.key.reset();
            }
        }
    }
}

SkillBoard::MagicIconButton::MagicIconButton(int argX, int argY, uint32_t argMagicID, SkillBoardConfig *configPtr, ProcessRun *proc, Widget *widgetPtr, bool autoDelete)
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

    , m_magicID([argMagicID]() -> uint32_t
      {
          fflassert(DBCOM_MAGICRECORD(argMagicID));
          fflassert(SkillBoard::getMagicIconGfx(argMagicID));
          return argMagicID;
      }())

    , m_config([configPtr]()
      {
          fflassert(configPtr);
          return configPtr;
      }())

    , m_processRun([proc]()
      {
          fflassert(proc);
          return proc;
      }())

    , m_icon
      {
          DIR_UPLEFT,
          0,
          0,

          {
              SkillBoard::getMagicIconGfx(argMagicID).magicIcon,
              SkillBoard::getMagicIconGfx(argMagicID).magicIcon,
              SkillBoard::getMagicIconGfx(argMagicID).magicIcon,
          },

          {
              0X01020000 + 103,
              SYS_U32NIL,
              SYS_U32NIL,
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
    if(const auto levelOpt = m_config->getMagicLevel(magicID()); levelOpt.has_value()){
        WidgetGroup::drawEx(dstX, dstY, srcX, srcY, srcW, srcH);

        const LabelBoard magicLevel
        {
            DIR_UPLEFT,
            0,
            0,
            str_printf(u8"%d", levelOpt.value()).c_str(),

            3,
            12,
            0,

            colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
        };
        magicLevel.drawAt(DIR_UPLEFT, (dstX - srcX) + (m_icon.w() - 2), (dstY - srcY) + (m_icon.h() - 1), dstX, dstY, srcW, srcH);

        if(const auto keyOpt = m_config->getMagicKey(magicID()); keyOpt.has_value()){
            const LabelShadowBoard magicKey
            {
                DIR_UPLEFT,
                0,
                0,
                2,
                2,
                str_printf(u8"%c", std::toupper(keyOpt.value())).c_str(),

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

SkillBoard::SkillPage::SkillPage(uint32_t pageImage, SkillBoardConfig *configPtr, ProcessRun *proc, Widget *widgetPtr, bool autoDelete)
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

    , m_config([configPtr]()
      {
          fflassert(configPtr);
          return configPtr;
      }())

    , m_processRun([proc]()
      {
          fflassert(proc);
          return proc;
      }())

    , m_pageImage(pageImage)
{
    std::tie(m_w, m_h) = [this]() -> std::tuple<int, int>
    {
        if(auto texPtr = g_progUseDB->retrieve(m_pageImage)){
            return SDLDeviceHelper::getTextureSize(texPtr);
        }

        const auto r = SkillBoard::getPageRectange();
        return {r[2], r[3]};
    }();
}

void SkillBoard::SkillPage::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(auto texPtr = g_progUseDB->retrieve(m_pageImage)){
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

SkillBoard::SkillBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget(DIR_UPLEFT, argX, argY, 0, 0, widgetPtr, autoDelete)
    , m_skillPageList([runPtr, this]() -> std::vector<SkillBoard::SkillPage *>
      {
          std::vector<SkillBoard::SkillPage *> pageList;
          pageList.reserve(8);

          for(int i = 0; i < 8; ++i){
              auto pagePtr = new SkillBoard::SkillPage
              {
                  to_u32(0X05000010 + to_u32(i)),
                  &m_config,
                  runPtr,
                  this,
                  true,
              };

              for(const auto &iconGfx: SkillBoard::getMagicIconGfxList()){
                  if(i == getSkillPageIndex(iconGfx.magicID)){
                      pagePtr->addIcon(iconGfx.magicID);
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
                      SYS_U32NIL,
                      0X05000020 + to_u32(i),
                      0X05000030 + to_u32(i),
                  },

                  {
                      SYS_U32NIL,
                      SYS_U32NIL,
                      0X01020000 + 105,
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
                          SYS_U32NIL,
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
    if(auto texPtr = g_progUseDB->retrieve(0X05000000)){
        std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid inventory frame texture");
    }
}

void SkillBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X05000000)){
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
            if(m_config->hasMagicID(magicID())){
                if(SkillBoard::getMagicIconGfx(magicID()).passive){
                    m_processRun->addCBLog(CBLOG_SYS, u8"无法为被动技能设置快捷键：%s", to_cstr(DBCOM_MAGICRECORD(magicID()).name));
                }
                else{
                    m_config->setMagicKey(magicID(), key);
                    m_processRun->requestSetMagicKey(magicID(), key);
                }
            }
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
        case SDL_MOUSEWHEEL:
            {
                auto pagePtr = m_skillPageList.at(m_selectedTabIndex);
                if(captureEvent && (r[3] < pagePtr->h())){
                    m_slider.addValue(event.wheel.y * -0.1f);
                    pagePtr->moveTo(r[0], r[1] - (pagePtr->h() - r[3]) * m_slider.getValue());
                }
                return focusConsume(this, true);
            }
        default:
            {
                return focusConsume(this, false);
            }
    }
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
                        if(const auto &mr = DBCOM_MAGICRECORD(magicIconPtr->magicID())){
                            return str_printf(u8"元素【%s】%s", str_haschar(mr.elem) ? to_cstr(mr.elem) : "无", to_cstr(mr.name));
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

        colorf::WHITE + colorf::A_SHF(255),
    };
    tabName.drawAt(DIR_UPLEFT, x() + 30, y() + 400);
}

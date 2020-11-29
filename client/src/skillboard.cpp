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
extern SDLDevice *g_SDLDevice;

SkillBoard::MagicIconButton::MagicIconButton(int argX, int argY, SkillBoard::MagicIconData *iconDataPtr, Widget *widgetPtr, bool autoDelete)
    : WidgetGroup
      {
          argX,
          argY,
          0,
          0,
          widgetPtr,
          autoDelete,
      }

    , m_key
      {
          2,
          2,
          u8"",

          3,
          20,
          0,

          colorf::RGBA(0XFF, 0X80, 0X00, 0X80),
          this,
      }

    , m_keyShadow
      {
          4,
          4,
          u8"",

          3,
          20,
          0,

          colorf::RGBA(0X00, 0X00, 0X00, 0X80),
          this,
      }

    , m_level
      {
          0,
          0,
          u8"",

          3,
          12,
          0,

          colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
          this,
      }

    , m_icon
      {
          0,
          0,

          {
              DBCOM_MAGICRECORD(iconDataPtr->magicID).icon,
              DBCOM_MAGICRECORD(iconDataPtr->magicID).icon,
              DBCOM_MAGICRECORD(iconDataPtr->magicID).icon,
          },

          [iconDataPtr, this]()
          {
              iconDataPtr->board->setText(str_printf(u8"元素【%s】%s", to_cstr(magicElemName(DBCOM_MAGICRECORD(iconDataPtr->magicID).elem)), to_cstr(DBCOM_MAGICRECORD(iconDataPtr->magicID).name)));
          },

          [iconDataPtr, this]()
          {
              iconDataPtr->board->setText(str_printf(u8"元素【%s】", to_cstr(magicElemName(iconDataPtr->board->selectedElem()))));
          },
          nullptr,

          0,
          0,
          0,
          0,

          false,
          this,
          true,
      }
{
    setKey(iconDataPtr->key);
    setLevel(iconDataPtr->level);

    m_level.moveTo(m_icon.w() - 2, m_icon.h() - 1);
    m_w = m_level.dx() + m_level.w();
    m_h = m_level.dy() + m_level.h();
}

void SkillBoard::MagicIconButton::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH)
{
    // to make sure the key always draw on top
    // otherwise WidgetGroup changes draw order when triggers icon callback, check WidgetGroup::drawEx()

    WidgetGroup::drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
    const auto fnDrawKey = [dstX, dstY, srcX, srcY, srcW, srcH, this](LabelBoard *b)
    {
        if(!b->show()){
            return;
        }

        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        if(!mathf::ROICrop(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    b->dx(), b->dy(), b->w(), b->h())){
            return;
        }
        b->drawEx(dstXCrop, dstYCrop, srcXCrop - b->dx(), srcYCrop - b->dy(), srcWCrop, srcHCrop);
    };

    fnDrawKey(&m_keyShadow);
    fnDrawKey(&m_key);
}

SkillBoard::SkillPage::SkillPage(uint32_t pageImage, Widget *widgetPtr, bool autoDelete)
    : WidgetGroup
      {
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
            return SDLDevice::getTextureSize(texPtr);
        }

        const auto r = SkillBoard::getPageRectange();
        return {r[2], r[3]};
    }();
}

void SkillBoard::SkillPage::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH)
{
    if(auto texPtr = g_progUseDB->Retrieve(m_pageImage)){
        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
        if(mathf::ROICrop(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    texW,
                    texH)){
            g_SDLDevice->drawTexture(texPtr, dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
        }
    }
    WidgetGroup::drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}

SkillBoard::SkillBoard(int nX, int nY, ProcessRun *pRun, Widget *pwidget, bool autoDelete)
    : Widget(nX, nY, 0, 0, pwidget, autoDelete)
    , m_magicIconDataList
      {
          {DBCOM_MAGICID(u8"雷电术"),   1,  12,  78, 'T', this},
          {DBCOM_MAGICID(u8"魔法盾"),   2, 252, 143, 'Y', this},
          {DBCOM_MAGICID(u8"召唤骷髅"), 3,  12,  13, 'U', this},
      }

    , m_skillPageList([this]() -> std::vector<SkillBoard::SkillPage *>
      {
          std::vector<SkillBoard::SkillPage *> pageList;
          pageList.reserve(8);

          for(int i = 0; i < 8; ++i){
              auto pagePtr = new SkillBoard::SkillPage
              {
                  to_u32(0X05000010 + (uint32_t)(i)),
                  this,
                  true,
              };

              for(auto &iconRef: m_magicIconDataList){
                  if(!iconRef.magicID){
                      continue;
                  }

                  const auto &mr = DBCOM_MAGICRECORD(iconRef.magicID);
                  const auto tabIndex = [&mr]() -> int
                  {
                      if(mr.elem == MET_NONE){
                          return 0;
                      }
                      else if(mr.elem < MET_END){
                          return mr.elem - 1;
                      }
                      else{
                          return -1;
                      }
                  }();

                  if(i == tabIndex){
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
                  tabX + tabW * i,
                  tabY,

                  {SYS_TEXNIL, 0X05000020 + (uint32_t)(i), 0X05000030 + (uint32_t)(i)},

                  [i, this]()
                  {
                      const int meType = i + 1;
                      if(meType >= MET_BEGIN && meType < MET_END){
                          m_textBoard.setText(u8"元素【%s】", to_cstr(magicElemName(i + 1)));
                      }
                      else{
                          m_textBoard.setText(u8"元素【无】");
                      }
                  },

                  [i, this]()
                  {
                      m_textBoard.setText(u8"元素【%s】", to_cstr(magicElemName(selectedElem())));
                  },

                  [i, this]()
                  {
                      if(m_tabIndex == i){
                          return;
                      }

                      m_tabButtonList.at(i)->setTexID(
                      {
                          0X05000030 + (uint32_t)(i),
                          0X05000030 + (uint32_t)(i),
                          0X05000030 + (uint32_t)(i),
                      });

                      m_tabButtonList.at(m_tabIndex)->setOff();
                      m_tabButtonList.at(m_tabIndex)->setTexID(
                      {
                          SYS_TEXNIL,
                          0X05000020 + (uint32_t)(m_tabIndex),
                          0X05000030 + (uint32_t)(m_tabIndex),
                      });

                      m_tabIndex = i;
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
                  this,
                  true,
              });

              if(i == m_tabIndex){
                  m_tabButtonList.at(i)->setTexID(
                  {
                      0X05000030 + (uint32_t)(i),
                      0X05000030 + (uint32_t)(i),
                      0X05000030 + (uint32_t)(i),
                  });
              }
          }
          return tabButtonList;
      }())

    , m_slider
      {
          326,
          74,

          266,
          2,

          [this](float value)
          {
              const auto r = SkillBoard::getPageRectange();
              auto pagePtr = m_skillPageList.at(m_tabIndex);

              if(r[3] < pagePtr->h()){
                  pagePtr->moveTo(r[0], r[1] - (pagePtr->h() - r[3]) * value);
              }
          },
          this,
      }

    , m_textBoard
      {
          30,
          400,
          u8"元素【火】", // default selected MET_FIRE

          1,
          12,
          0,

          colorf::WHITE + 255,
          this,
      }

    , m_closeButton
      {
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
          this,
      }
    , m_processRun(pRun)
{
    show(false);
    if(auto texPtr = g_progUseDB->Retrieve(0X05000000)){
        std::tie(m_w, m_h) = SDLDevice::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid inventory frame texture");
    }
}

void SkillBoard::drawEx(int dstX, int dstY, int, int, int, int)
{
    if(auto texPtr = g_progUseDB->Retrieve(0X05000000)){
        g_SDLDevice->drawTexture(texPtr, dstX, dstY);
    }

    m_slider.draw();
    m_textBoard.draw();
    m_closeButton.draw();

    for(auto buttonPtr: m_tabButtonList){
        buttonPtr->draw();
    }

    const auto r = SkillBoard::getPageRectange();
    auto pagePtr = m_skillPageList.at(m_tabIndex);
    pagePtr->drawEx(dstX + r[0], dstY + r[1], r[0] - pagePtr->dx(), r[1] - pagePtr->dy(), r[2], r[3]);
}

void SkillBoard::update(double)
{
}

bool SkillBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return focusConsumer(this, false);
    }

    if(!show()){
        return focusConsumer(this, false);
    }

    if(m_closeButton.processEvent(event, valid)){
        return focusConsumer(this, show());
    }

    bool tabConsumed = false;
    for(auto buttonPtr: m_tabButtonList){
        tabConsumed |= buttonPtr->processEvent(event, valid && !tabConsumed);
    }

    if(tabConsumed){
        return focusConsumer(this, true);
    }

    if(m_slider.processEvent(event, valid)){
        return focusConsumer(this, true);
    }

    const auto r = getPageRectange();
    const auto loc = SDLDevice::getEventLocation(event);
    const bool captureEvent = loc && mathf::pointInRectangle(loc.x, loc.y, x() + r[0], y() + r[1], r[2], r[3]);

    if(m_skillPageList.at(m_tabIndex)->processEvent(event, captureEvent && valid)){
        return focusConsumer(this, true);
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y) || focus())){
                    const auto [rendererW, rendererH] = g_SDLDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, x() + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, y() + event.motion.yrel));

                    moveBy(newX - x(), newY - y());
                    return focusConsumer(this, true);
                }
                return focusConsumer(this, false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            return focusConsumer(this, in(event.button.x, event.button.y));
                        }
                    default:
                        {
                            return focusConsumer(this, false);
                        }
                }
            }
        default:
            {
                return focusConsumer(this, false);
            }
    }
}

uint32_t SkillBoard::key2MagicID(char key)
{
    for(const auto &iconCRef: m_magicIconDataList){
        if(std::tolower(iconCRef.key) == std::tolower(key)){
            return iconCRef.magicID;
        }
    }
    return 0;
}

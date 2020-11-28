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

SkillBoard::SkillBoard(int nX, int nY, ProcessRun *pRun, Widget *pwidget, bool autoDelete)
    : Widget(nX, nY, 0, 0, pwidget, autoDelete)
    , m_magicIconDataList
      {
          {DBCOM_MAGICID(u8"雷电术"),   1,  12,  78, 'T'},
          {DBCOM_MAGICID(u8"魔法盾"),   2, 252, 143, 'Y'},
          {DBCOM_MAGICID(u8"召唤骷髅"), 3,  12,  13, 'U'},
      }

    , m_skillPageList([this]() -> std::vector<SkillBoard::SkillPage *>
      {
          std::vector<SkillBoard::SkillPage *> pageList;
          pageList.reserve(8);

          for(int i = 0; i < 8; ++i){
              auto pagePtr = new SkillBoard::SkillPage
              {
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

              // need to push background image at last
              // otherwise it hide all buttons, check draw order of WidgetGroup::drawEx()
              pagePtr->setPageImage(0X05000010 + (uint32_t)(i));
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
                      m_textBoard.setText(u8"元素");
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
              const int pageImageHeight = m_skillPageList.at(m_tabIndex)->pageImageSize().at(1);

              if(r[3] < pageImageHeight){
                  m_skillPageList.at(m_tabIndex)->moveTo(r[0], r[1] - (pageImageHeight - r[3]) * value);
              }
          },
          this,
      }

    , m_textBoard
      {
          30,
          400,
          u8"元素",

          1,
          12,
          0,

          colorf::WHITE,
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

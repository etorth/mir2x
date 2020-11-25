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

#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "skillboard.hpp"
#include "tritexbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

SkillBoard::SkillBoard(int nX, int nY, ProcessRun *pRun, Widget *pwidget, bool autoDelete)
    : Widget(nX, nY, 0, 0, pwidget, autoDelete)
    , m_tabButtonList([this]() -> std::vector<std::array<std::unique_ptr<TritexButton>, 2>>
      {
          std::vector<std::array<std::unique_ptr<TritexButton>, 2>> tabButtonList;
          tabButtonList.reserve(8);

          const int tabX = 45;
          const int tabY = 10;
          const int tabW = 34;

          // don't know WTF
          // can't compile if use std::vector<TritexButton>
          // looks for complicated parameter list the forward can't be done correctly ???

          for(int i = 0; i < 8; ++i){
              tabButtonList.emplace_back(std::array<std::unique_ptr<TritexButton>, 2>
              {
                  std::unique_ptr<TritexButton>(new TritexButton // inactive
                  {
                      tabX + tabW * i,
                      tabY,

                      {SYS_TEXNIL, 0X05000020 + (uint32_t)(i), 0X05000030 + (uint32_t)(i)},

                      nullptr,
                      [i, this]()
                      {
                          m_tabButtonList.at(i)         .at(0)->setOff();
                          m_tabButtonList.at(m_tabIndex).at(0)->setOff();
                          m_tabIndex = i;
                      },

                      0,
                      0,
                      0,
                      0,

                      false,
                      this,
                      false,
                  }),

                  std::unique_ptr<TritexButton>(new TritexButton // active
                  {
                      tabX + tabW * i,
                      tabY,

                      {0X05000030 + (uint32_t)(i), 0X05000030 + (uint32_t)(i), 0X05000030 + (uint32_t)(i)},

                      nullptr,
                      nullptr,

                      0,
                      0,
                      0,
                      0,

                      false,
                      this,
                      false,
                  }),
              });
          }
          return tabButtonList;
      }())

    , m_closeButton
      {
          317,
          402,
          {SYS_TEXNIL, 0X0000001C, 0X0000001D},

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

void SkillBoard::update(double)
{
}

void SkillBoard::drawEx(int dstX, int dstY, int, int, int, int)
{
    if(auto texPtr = g_progUseDB->Retrieve(0X05000000)){
        g_SDLDevice->DrawTexture(texPtr, dstX, dstY);
    }

    m_closeButton.draw();
    for(int i = 0; i < (int)(m_tabButtonList.size()); ++i){
        m_tabButtonList.at(i).at(i == m_tabIndex)->draw();
    }
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
    const int lastTabIndex = m_tabIndex; // may change in callback

    for(int i = 0; i < (int)(m_tabButtonList.size()); ++i){
        tabConsumed |= m_tabButtonList.at(i).at(i == lastTabIndex)->processEvent(event, valid && !tabConsumed);
    }

    if(tabConsumed){
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

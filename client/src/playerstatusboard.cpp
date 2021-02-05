/*
 * =====================================================================================
 *
 *       Filename: playerstatusboard.cpp
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

#include <type_traits>
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "pngtexoffdb.hpp"
#include "inventoryboard.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_equipDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

PlayerStatusBoard::PlayerStatusBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          argX,
          argY,
          0,
          0,

          widgetPtr,
          autoDelete
      }
    , m_gridList([this]()
      {
          std::remove_cvref_t<decltype(this->m_gridList)> gridList;
          gridList.fill({});

          gridList[GRID_DRESS] = WearGrid
          {
              .x = m_equipCharX -   0,
              .y = m_equipCharY - 100,
              .w =  60,
              .h = 110,
          };

          gridList[GRID_HELMET] = WearGrid
          {
              .x = m_equipCharX +  15,
              .y = m_equipCharY - 130,
              .w = 20,
              .h = 15,
          };

          gridList[GRID_WEAPON] = WearGrid
          {
              .x = m_equipCharX -  50,
              .y = m_equipCharY - 120,
              .w = 45,
              .h = 90,
          };

          gridList[GRID_SHOES] = WearGrid
          {
              .x =  10,
              .y = 235,
              .h =  67,
          };

          gridList[GRID_NECKLACE ] = WearGrid{.x = 168, .y =  88, };
          gridList[GRID_ARMRING_L] = WearGrid{.x =  10, .y = 155, };
          gridList[GRID_ARMRING_R] = WearGrid{.x = 168, .y = 155, };
          gridList[GRID_RING_L   ] = WearGrid{.x =  10, .y = 195, };
          gridList[GRID_RING_R   ] = WearGrid{.x = 168, .y = 195, };
          gridList[GRID_TORCH    ] = WearGrid{.x =  90, .y = 265, };
          gridList[GRID_CHARM    ] = WearGrid{.x = 130, .y = 265, };

          return gridList;
      }())

    , m_closeButton
      {
          288,
          13,
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
          false,
      }
    , m_processRun(runPtr)
{
    show(false);
    if(auto texPtr = g_progUseDB->Retrieve(0X06000000)){
        std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid player status board frame texture");
    }

    for(int r: {0, 1, 2}){
        for(int i = 0; i < 7; ++i){
            m_elemStatusList.push_back(new TritexButton
            {
                62  + i * 37,
                374 + r * 30,

                {
                    SYS_TEXNIL,
                    0X06000010 + (uint32_t)(i),
                    0X06000020 + (uint32_t)(i),
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
                true,
            });
        }
    }
}

void PlayerStatusBoard::update(double)
{
}

void PlayerStatusBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->Retrieve(0X06000000)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }

    const auto myHeroPtr = m_processRun->getMyHero();
    if(auto [texPtr, dx, dy] = g_equipDB->Retrieve(myHeroPtr->Gender() ? 0X00000000 : 0X00000001); texPtr){
        g_sdlDevice->drawTexture(texPtr, dstX + m_equipCharX + dx, dstY + m_equipCharY + dy);
    }

    if(const auto dressItemID = myHeroPtr->Dress()){
        if(const auto useGfxIndex = DBCOM_ITEMRECORD(dressItemID).useGfxID; useGfxIndex > 0){
            if(auto [texPtr, dx, dy] = g_equipDB->Retrieve((myHeroPtr->Gender() ? 0X010003AC : 0X010003B6) + useGfxIndex - 1); texPtr){
                g_sdlDevice->drawTexture(texPtr, dstX + m_equipCharX + dx, dstY + m_equipCharY + dy);
            }
        }
    }

    if(const auto weaponItemID = myHeroPtr->Weapon()){
        if(const auto useGfxIndex = DBCOM_ITEMRECORD(weaponItemID).useGfxID; useGfxIndex > 0){
            if(auto [texPtr, dx, dy] = g_equipDB->Retrieve(0X01000000 + DBCOM_ITEMRECORD(weaponItemID).pkgGfxID); texPtr){
                g_sdlDevice->drawTexture(texPtr, dstX + m_equipCharX + dx, dstY + m_equipCharY + dy);
            }
        }
    }

    if(const auto helmetItemID = myHeroPtr->helmet()){
        if(const auto useGfxIndex = DBCOM_ITEMRECORD(helmetItemID).useGfxID; useGfxIndex > 0){
            if(auto [texPtr, dx, dy] = g_equipDB->Retrieve(0X01000000 + DBCOM_ITEMRECORD(helmetItemID).pkgGfxID); texPtr){
                g_sdlDevice->drawTexture(texPtr, dstX + m_equipCharX + dx, dstY + m_equipCharY + dy);
            }
        }
    }
    else{
        if(myHeroPtr->hair() >= HAIR_BEGIN){
            if(auto [texPtr, dx, dy] = g_equipDB->Retrieve((myHeroPtr->Gender() ? 0X0000003C : 0X00000046) + myHeroPtr->hair() - HAIR_BEGIN); texPtr){
                SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, myHeroPtr->hairColor());
                g_sdlDevice->drawTexture(texPtr, dstX + m_equipCharX + dx, dstY + m_equipCharY + dy);
            }
        }
    }

    m_closeButton.draw();
    for(auto buttonPtr: m_elemStatusList){
        buttonPtr->draw();
    }

    if(g_clientArgParser->debugPlayerStatusBoard){
        for(size_t i = GRID_BEGIN; i < GRID_END; ++i){
            g_sdlDevice->drawRectangle(colorf::BLUE + 255, x() + m_gridList[i].x, y() + m_gridList[i].y, m_gridList[i].w, m_gridList[i].h);
        }
    }
}

bool PlayerStatusBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return focusConsume(this, false);
    }

    if(!show()){
        return focusConsume(this, false);
    }

    bool consumed = false;
    for(auto buttonPtr: m_elemStatusList){
        consumed |= buttonPtr->processEvent(event, !consumed && valid);
    }

    if(consumed){
        return focusConsume(this, false);
    }

    if(m_closeButton.processEvent(event, valid)){
        return true;
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

/*
 * =====================================================================================
 *
 *       Filename: quickaccessboard.cpp
 *        Created: 03/28/2020 05:43:45
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

#include <tuple>
#include "totype.hpp"
#include "pngtexdb.hpp"
#include "sysconst.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "quickaccessboard.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

QuickAccessBoard::QuickAccessBoard(int x, int y, ProcessRun *proc, Widget *pwidget, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          x,
          y,
          0,
          0,
          pwidget,
          autoDelete,
      }

    , m_processRun(proc)
    , m_buttonClose
      {
          DIR_UPLEFT,
          263,
          32,
          {SYS_TEXNIL, 0X00000061, 0X00000062},

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
          this
      }
{
    auto texPtr = g_progUseDB->retrieve(m_texID);
    if(!texPtr){
        throw fflerror("no valid quick access board texture: texID = %llu", to_llu(m_texID));
    }

    show(false);
    std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
}

void QuickAccessBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(m_texID)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }

    m_buttonClose.drawEx(dstX + m_buttonClose.dx(), dstY + m_buttonClose.dy(), 0, 0, m_buttonClose.w(), m_buttonClose.h());
    for(int slot = 0; slot < 6; ++slot){
        const auto &item = m_processRun->getMyHero()->getBelt(slot);
        if(!item){
            continue;
        }

        const auto [gridX, gridY, gridW, gridH] = getGridLoc(slot);
        if(auto texPtr = g_itemDB->retrieve(DBCOM_ITEMRECORD(item.itemID).pkgGfxID | 0X01000000)){
            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
            const auto drawDstX = x() + gridX + (gridW - texW) / 2;
            const auto drawDstY = y() + gridY + (gridH - texH) / 2;
            g_sdlDevice->drawTexture(texPtr, drawDstX, drawDstY);
            if(item.count > 1){
                LabelBoard
                {
                    DIR_UPRIGHT,
                    x() + gridX + gridW,
                    y() + gridY,
                    to_u8cstr(std::to_string(item.count)),

                    1,
                    10,
                    0,

                    colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
                }.draw();
            }
        }
    }

    const auto [mouseX, mouseY] = SDLDeviceHelper::getMousePLoc();
    for(int slot = 0; slot < 6; ++slot){
        const auto [gridX, gridY, gridW, gridH] = getGridLoc(slot);
        const auto gridStartX = x() + gridX;
        const auto gridStartY = y() + gridY;
        if(mathf::pointInRectangle(mouseX, mouseY, gridStartX, gridStartY, gridW, gridH)){
            g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(64), gridStartX, gridStartY, gridW, gridH);
            break;
        }
    }
}

bool QuickAccessBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        focus(false);
        return false;
    }

    if(!show()){
        return false;
    }

    if(m_buttonClose.processEvent(event, valid)){
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

                    focus(true);
                    return true;
                }

                focus(false);
                return false;
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            auto &beltRef = m_processRun->getMyHero()->getBelt();
                            auto &invPackRef = m_processRun->getMyHero()->getInvPack();
                            for(int i = 0; i < 6; ++i){
                                const auto [gridX, gridY, gridW, gridH] = getGridLoc(i);
                                if(mathf::pointInRectangle(event.button.x, event.button.y, x() + gridX, y() + gridY, gridW, gridH)){
                                    if(const auto grabbedItem = invPackRef.getGrabbedItem()){
                                        const auto &ir = DBCOM_ITEMRECORD(grabbedItem.itemID);
                                        if(ir.beltable()){
                                            m_processRun->requestEquipBelt(grabbedItem.itemID, grabbedItem.seqID, i);
                                        }
                                        else{
                                            invPackRef.add(grabbedItem);
                                            invPackRef.setGrabbedItem({});
                                        }
                                    }
                                    else if(beltRef.list.at(i)){
                                        m_processRun->requestGrabBelt(i);
                                    }
                                    break;
                                }
                            }

                            if(in(event.button.x, event.button.y)){
                                focus(true);
                                return true;
                            }
                            else{
                                focus(false);
                                return false;
                            }
                        }
                    default:
                        {
                            focus(false);
                            return false;
                        }
                }
            }
        default:
            {
                return false;
            }
    }
}

/*
 * =====================================================================================
 *
 *       Filename: guimanager.cpp
 *        Created: 08/12/2015 09:59:15
 *    Description: public API for class client only
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

#include "fflerror.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "guimanager.hpp"

extern SDLDevice *g_sdlDevice;

GUIManager::GUIManager(ProcessRun *proc)
    : WidgetGroup
      {
          DIR_UPLEFT,
          0,
          0,
          g_sdlDevice->getRendererWidth(),
          g_sdlDevice->getRendererHeight(),
      }

    , m_proc(proc)
    , m_NPCChatBoard
      {
          proc,
      }

    , m_controlBoard
      {
          g_sdlDevice->getRendererWidth(),
          g_sdlDevice->getRendererHeight() - 133,
          proc,
      }

    , m_skillBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 180,
          g_sdlDevice->getRendererHeight() / 2 - 224,
          proc,
          this,
      }

    , m_miniMapBoard
      {
          proc,
      }

    , m_purchaseBoard
      {
          proc,
          this,
      }

    , m_inventoryBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 141,
          g_sdlDevice->getRendererHeight() / 2 - 233,
          proc,
          this,
      }

    , m_quickAccessBoard
      {
          0,
          g_sdlDevice->getRendererHeight() - m_controlBoard.h() - 48,
          proc,
          this,
      }

    , m_playerStateBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 164,
          g_sdlDevice->getRendererHeight() / 2 - 233,
          proc,
          this,
      }

    , m_inputStringBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 179,
          g_sdlDevice->getRendererHeight() / 2 - 134,
          proc,
          this,
      }
{
    if(!m_proc){
        throw fflerror("null ProcessRun pointer");
    }
}

void GUIManager::drawEx(int, int, int, int, int, int) const
{
    m_miniMapBoard .draw();
    m_NPCChatBoard .draw();
    m_controlBoard .draw();
    m_purchaseBoard.draw();

    const auto [w, h] = g_sdlDevice->getRendererSize();
    WidgetGroup::drawEx(0, 0, 0, 0, w, h);
}

void GUIManager::update(double fUpdateTime)
{
    WidgetGroup  ::update(fUpdateTime);
    m_controlBoard.update(fUpdateTime);
    m_NPCChatBoard.update(fUpdateTime);
}

bool GUIManager::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        throw fflerror("event passed to GUIManager is not valid");
    }

    switch(event.type){
        case SDL_WINDOWEVENT:
            {
                switch(event.window.event){
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            onWindowResize();
                            return true;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        default:
            {
                break;
            }
    }

    bool tookEvent = false;
    tookEvent |= WidgetGroup  ::processEvent(event, valid && !tookEvent);
    tookEvent |= m_controlBoard.processEvent(event, valid && !tookEvent);
    tookEvent |= m_NPCChatBoard.processEvent(event, valid && !tookEvent);
    tookEvent |= m_miniMapBoard.processEvent(event, valid && !tookEvent);

    return tookEvent;
}

Widget *GUIManager::getWidget(const std::string &widgetName)
{
    if(widgetName == "InventoryBoard"){
        return &m_inventoryBoard;
    }

    if(widgetName == "QuickAccessBoard"){
        return &m_quickAccessBoard;
    }

    if(widgetName == "NPCChatBoard"){
        return &m_NPCChatBoard;
    }

    if(widgetName == "ControlBoard"){
        return &m_controlBoard;
    }

    if(widgetName == "SkillBoard"){
        return &m_skillBoard;
    }

    if(widgetName == "MiniMapBoard"){
        return &m_miniMapBoard;
    }

    if(widgetName == "PlayerStateBoard"){
        return &m_playerStateBoard;
    }

    if(widgetName == "PurchaseBoard"){
        return &m_purchaseBoard;
    }

    if(widgetName == "InputStringBoard"){
        return &m_inputStringBoard;
    }

    return nullptr;
}

void GUIManager::onWindowResize()
{
    std::tie(m_w, m_h) = g_sdlDevice->getRendererSize();

    m_controlBoard.onWindowResize(w(), h());
    m_controlBoard.moveTo(0, h() - 133);

    if(m_miniMapBoard.getMiniMapTexture()){
        m_miniMapBoard.setPLoc();
    }

    const auto fnSetWidgetPLoc = [this](Widget *widgetPtr)
    {
        const auto moveDX = std::max<int>(widgetPtr->x() - (m_w - widgetPtr->w()), 0);
        const auto moveDY = std::max<int>(widgetPtr->y() - (m_h - widgetPtr->h()), 0);

        // move upper-left
        //
        widgetPtr->moveBy(-moveDX, -moveDY);
    };

    fnSetWidgetPLoc(&m_skillBoard);
    fnSetWidgetPLoc(&m_inventoryBoard);
    fnSetWidgetPLoc(&m_quickAccessBoard);
    fnSetWidgetPLoc(&m_playerStateBoard);
    fnSetWidgetPLoc(&m_inputStringBoard);
}

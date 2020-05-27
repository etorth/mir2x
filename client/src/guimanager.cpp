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

extern SDLDevice *g_SDLDevice;

GUIManager::GUIManager(ProcessRun *proc)
    : WidgetGroup
      {
          0,
          0,
          g_SDLDevice->getRendererWidth(),
          g_SDLDevice->getRendererHeight(),
      }

    , m_proc(proc)
    , m_NPCChatBoard
      {
          proc,
      }

    , m_controlBoard
      {
          g_SDLDevice->getRendererWidth(),
          g_SDLDevice->getRendererHeight() - 133,
          proc,
      }

    , m_inventoryBoard
      {
          g_SDLDevice->getRendererWidth()  / 2 - 141,
          g_SDLDevice->getRendererHeight() / 2 - 233,
          proc,
          this,
      }

    , m_quickAccessBoard
      {
          0,
          g_SDLDevice->getRendererHeight() - m_controlBoard.h(),
          proc,
          this,
      }
{
    if(!m_proc){
        throw fflerror("null ProcessRun pointer");
    }
}

void GUIManager::drawEx(int, int, int, int, int, int)
{
    m_NPCChatBoard.draw();
    m_controlBoard.draw();

    const auto [w, h] = g_SDLDevice->getRendererSize();
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
                            const auto [w, h] = g_SDLDevice->getRendererSize();
                            m_controlBoard.resizeWidth(w);
                            m_controlBoard.moveTo(0, h - 133);
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

    return nullptr;
}

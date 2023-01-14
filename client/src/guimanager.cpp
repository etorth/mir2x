#include "fflerror.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "guimanager.hpp"

extern SDLDevice *g_sdlDevice;

GUIManager::GUIManager(ProcessRun *proc)
    : WidgetContainer
      {
          DIR_UPLEFT,
          0,
          0,
          g_sdlDevice->getRendererWidth(),
          g_sdlDevice->getRendererHeight(),
      }

    , m_processRun(proc)
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
          DIR_UPLEFT,
          g_sdlDevice->getRendererWidth()  / 2 - 179,
          g_sdlDevice->getRendererHeight() / 2 - 134,
          false,
          this,
      }

    , m_runtimeConfigBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 255,
          g_sdlDevice->getRendererHeight() / 2 - 234,
          proc,
          this,
      }

    , m_securedItemListBoard
      {
          0,
          0,
          m_processRun,
          this,
      }
{
    fflassert(m_processRun);
}

void GUIManager::drawEx(int, int, int, int, int, int) const
{
    m_miniMapBoard .draw();
    m_NPCChatBoard .draw();
    m_controlBoard .draw();
    m_purchaseBoard.draw();

    const auto [w, h] = g_sdlDevice->getRendererSize();
    WidgetContainer::drawEx(0, 0, 0, 0, w, h);
}

void GUIManager::update(double fUpdateTime)
{
    WidgetContainer::update(fUpdateTime);
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
    tookEvent |= WidgetContainer::processEvent(event, valid && !tookEvent);
    tookEvent |= m_controlBoard.processEvent(event, valid && !tookEvent);
    tookEvent |= m_NPCChatBoard.processEvent(event, valid && !tookEvent);
    tookEvent |= m_miniMapBoard.processEvent(event, valid && !tookEvent);

    return tookEvent;
}

Widget *GUIManager::getWidget(const std::string &name)
{
    if(name == "InventoryBoard"){
        return &m_inventoryBoard;
    }

    else if(name == "QuickAccessBoard"){
        return &m_quickAccessBoard;
    }

    else if(name == "NPCChatBoard"){
        return &m_NPCChatBoard;
    }

    else if(name == "ControlBoard"){
        return &m_controlBoard;
    }

    else if(name == "SkillBoard"){
        return &m_skillBoard;
    }

    else if(name == "MiniMapBoard"){
        return &m_miniMapBoard;
    }

    else if(name == "PlayerStateBoard"){
        return &m_playerStateBoard;
    }

    else if(name == "PurchaseBoard"){
        return &m_purchaseBoard;
    }

    else if(name == "InputStringBoard"){
        return &m_inputStringBoard;
    }

    else if(name == "RuntimeConfigBoard"){
        return &m_runtimeConfigBoard;
    }

    else if(name == "SecuredItemListBoard"){
        return &m_securedItemListBoard;
    }

    else{
        throw fflvalue(name);
    }
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

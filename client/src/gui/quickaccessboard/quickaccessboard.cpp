#include <tuple>
#include "totype.hpp"
#include "invpack.hpp"
#include "pngtexdb.hpp"
#include "sysconst.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "widget.hpp"
#include "quickaccessgrid.hpp"
#include "quickaccessboard.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

QuickAccessBoard::QuickAccessBoard(
        dir8_t argDir,

        int argX,
        int argY,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,

          .w = std::nullopt,
          .h = std::nullopt,

          .attrs
          {
              .inst
              {
                  .show = false, // hide by default
              },
          },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_bg
      {{
          .texLoadFunc = [this]{ return g_progUseDB->retrieve(m_texID); },
          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = 263,
          .y = 32,

          .texIDList
          {
              .off  = 0X00000061,
              .on   = 0X00000061,
              .down = 0X00000062,
          },

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
          },

          .parent{this},
      }}
{
    for(int slot = 0; slot < 6; ++slot){
        addChild(new QuickAccessGrid
        {
            DIR_UPLEFT,

            getGridLoc(slot).x,
            getGridLoc(slot).y,
            getGridLoc(slot).w,
            getGridLoc(slot).h,

            slot,
            m_processRun,
        },

        true);
    }
}

bool QuickAccessBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_buttonClose.processEventParent(event, valid, m)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || focus())){
                    moveBy(event.motion.xrel, event.motion.yrel, Widget::makeROI(0, 0, g_sdlDevice->getRendererSize()));
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            for(int slot = 0; slot < 6; ++slot){
                                const auto [gridX, gridY, gridW, gridH] = getGridLoc(slot);
                                if(mathf::pointInRectangle(event.button.x, event.button.y, m.x + gridX, m.y + gridY, gridW, gridH)){
                                    if(const auto grabbedItem = m_processRun->getMyHero()->getInvPack().getGrabbedItem()){
                                        const auto &ir = DBCOM_ITEMRECORD(grabbedItem.itemID);
                                        if(ir.beltable()){
                                            m_processRun->requestEquipBelt(grabbedItem.itemID, grabbedItem.seqID, slot);
                                        }
                                        else{
                                            m_processRun->getMyHero()->getInvPack().add(grabbedItem);
                                            m_processRun->getMyHero()->getInvPack().setGrabbedItem({});
                                        }
                                    }
                                    else if(m_processRun->getMyHero()->getBelt(slot)){
                                        m_processRun->requestGrabBelt(slot);
                                    }
                                    break;
                                }
                            }

                            if(m.in(event.button.x, event.button.y)){
                                return consumeFocus(true);
                            }
                            else{
                                return consumeFocus(false);
                            }
                        }
                    case SDL_BUTTON_RIGHT:
                        {
                            for(int slot = 0; slot < 6; ++slot){
                                const auto [gridX, gridY, gridW, gridH] = getGridLoc(slot);
                                if(mathf::pointInRectangle(event.button.x, event.button.y, m.x + gridX, m.y + gridY, gridW, gridH)){
                                    gridConsume(slot);
                                    break;
                                }
                            }

                            if(m.in(event.button.x, event.button.y)){
                                return consumeFocus(true);
                            }
                            else{
                                return consumeFocus(false);
                            }
                        }
                    default:
                        {
                            return consumeFocus(false);
                        }
                }
            }
        case SDL_KEYDOWN:
            {
                if(focus()){
                    if(const auto ch = SDLDeviceHelper::getKeyChar(event, false); ch >= '1' && ch <= '6'){
                        gridConsume(ch - '1');
                    }
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        default:
            {
                return false;
            }
    }
}

void QuickAccessBoard::gridConsume(int slot)
{
    fflassert(slot >= 0, slot);
    fflassert(slot <  6, slot);

    if(const auto &item = m_processRun->getMyHero()->getBelt(slot)){
        InvPack::playItemSoundEffect(item.itemID, true);
        m_processRun->requestConsumeItem(item.itemID, item.seqID, 1);
    }
}

/*
 * =====================================================================================
 *
 *       Filename: purchasecountboard.cpp
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
#include "purchasecountboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

PurchaseCountBoard::PurchaseCountBoard(int x, int y, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          x,
          y,
          0,
          0,

          widgetPtr,
          autoDelete
      }

    , m_input
      {
          22,
          225,
          315,
          23,

          1,
          14,

          0,
          colorf::WHITE + 255,

          2,
          colorf::WHITE + 255,

          nullptr,
          [this]()
          {
          },

          this,
          false,
      }

    , m_yesButton
      {
          66,
          190,
          {0X07000001, 0X07000002, 0X07000003},

          nullptr,
          nullptr,
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
          false,
          this,
          false,
      }

    , m_nopButton
      {
          212,
          190,
          {0X07000004, 0X07000005, 0X07000006},

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
          false,
          this,
          false,
      }

    , m_processRun(runPtr)
{
    show(false);
    if(auto texPtr = g_progUseDB->Retrieve(0X07000000)){
        std::tie(m_w, m_h) = SDLDevice::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid purchase count board frame texture");
    }
}

void PurchaseCountBoard::update(double ms)
{
    m_input.update(ms);
}

void PurchaseCountBoard::drawEx(int dstX, int dstY, int, int, int, int)
{
    if(auto texPtr = g_progUseDB->Retrieve(0X07000000)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }

    m_input    .draw();
    m_yesButton.draw();
    m_nopButton.draw();

    if(m_input.focus()){
        g_sdlDevice->fillRectangle(colorf::WHITE + 32, m_input.x(), m_input.y(), m_input.w(), m_input.h());
    }
}

bool PurchaseCountBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return focusConsume(this, false);
    }

    if(!show()){
        return focusConsume(this, false);
    }

    if(m_input.processEvent(event, valid)){
        return true;
    }

    if(m_yesButton.processEvent(event, valid)){
        return true;
    }

    if(m_nopButton.processEvent(event, valid)){
        return true;
    }
    return true;
}

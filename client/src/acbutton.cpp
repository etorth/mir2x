/*
 * =====================================================================================
 *
 *       Filename: acbutton.cpp
 *        Created: 08/25/2016 04:12:57
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

#include "widget.hpp"
#include "acbutton.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

ACButton::ACButton(int x, int y, ProcessRun *proc, const std::vector<std::string> &buttonList, Widget *pwidget, bool autoDelete)
    : ButtonBase
      {
          x,
          y,
          0,
          0,

          nullptr,
          [this]()
          {
              m_currButtonName = (m_currButtonName + 1) % m_buttonNameList.size();
              setLabel();
          },

          0,
          0,
          0,
          0,

          false,
          pwidget,
          autoDelete,
      }
    , m_proc(proc)
    , m_texMap
      {
          {"AC", 0X00000046},
          {"DC", 0X00000047},
          {"MA", 0X00000048},
          {"MC", 0X00000049},
      }
    , m_currButtonName(0)
    , m_buttonNameList(buttonList)
    , m_labelBoard
      {
          0,
          0,
          u8"0-0",

          1,
          12,
          0,

          colorf::RGBA(0XFF, 0XFF, 0X00, 0X00),
          this,
      }
{
    if(!m_proc){
        throw fflerror("null process pointer");
    }

    if(m_buttonNameList.empty()){
        throw fflerror("button list empty");
    }

    bool inited = false;
    for(auto &p: m_texMap){
        if(auto texPtr = g_progUseDB->Retrieve(p.second)){
            const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
            m_w = std::max<int>(m_w, texW);
            m_h = std::max<int>(m_h, texH);
            inited = true;
        }
    }

    if(!inited){
        throw fflerror("missing texture for ACButton");
    }

    setLabel();
}

void ACButton::drawEx(int dstX, int dstY, int, int, int, int)
{
    const auto buttonName = m_buttonNameList.at(m_currButtonName);
    auto texPtr = g_progUseDB->Retrieve(m_texMap.at(buttonName));

    if(!texPtr){
        throw fflerror("no texture for %s", buttonName.c_str());
    }

    SDL_SetTextureAlphaMod(texPtr, 255);
    SDL_SetTextureColorMod(texPtr, 255, 255, 255);

    g_SDLDevice->DrawTexture(texPtr, dstX, dstY);
    m_labelBoard.drawEx(dstX + w() + 5, dstY, 0, 0, m_labelBoard.w(), m_labelBoard.h());

    switch(m_state){
        case BEVENT_ON:
        case BEVENT_DOWN:
            {
                SDL_SetTextureColorMod(texPtr, 255, 0, 0);
                SDL_SetTextureAlphaMod(texPtr, 128);
                g_SDLDevice->DrawTexture(texPtr, dstX, dstY);
                break;
            }
        default:
            {
                break;
            }
    }
}

void ACButton::setLabel()
{
    const auto buttonName = m_buttonNameList.at(m_currButtonName);
    const auto [low, high] = m_proc->getACNum(buttonName);

    if(low > high){
        throw fflerror("invalid %s: %d-%d", to_cstr(buttonName), low, high);
    }
    m_labelBoard.setText(u8"%d-%d", low, high);
}

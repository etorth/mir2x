/*
 * =====================================================================================
 *
 *       Filename: mmapboard.cpp
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

#include <array>
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "mmapboard.hpp"
#include "maprecord.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"

extern Client *g_client;
extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

MMapBoard::MMapBoard(ProcessRun *runPtr, Widget *parent, bool autoDelete)
    : Widget
      {
          0,
          0,
          0,
          0,

          parent,
          autoDelete,
      }
    , m_processRun(runPtr)
    , m_buttonAlpha
      {
          0,
          0,

          {
              0X09000002,
              0X09000002,
              0X09000003,
          },

          nullptr,
          nullptr,
          [this]()
          {
              m_alphaOn = !m_alphaOn;
          },

          0,
          0,
          0,
          0,

          false,
          true,
          this,
      }
    , m_buttonExtend
      {
          0,
          0,

          {
              0X09000004,
              0X09000004,
              0X09000005,
          },

          nullptr,
          nullptr,
          [this]()
          {
              switch(m_state){
                  case MMAP_ON:
                      {
                          setState(MMAP_EXTENDED);
                          return;
                      }
                  case MMAP_EXTENDED:
                      {
                          setState(MMAP_ON);
                          return;
                      }
                  default:
                      {
                          throw bad_reach();
                      }
              }
          },

          0,
          0,
          0,
          0,

          false,
          true,
          this,
      }
{
    setLoc();
}

void MMapBoard::drawEx(int, int, int, int, int, int) const
{
   switch(m_state){
        case MMAP_OFF:
            {
                return;
            }
        case MMAP_ON:
        case MMAP_EXTENDED:
            {
                if(auto texPtr = g_progUseDB->Retrieve(m_state == MMAP_ON ? 0X09000000 : 0X09000001); texPtr){
                    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                    if(!m_alphaOn){
                        g_sdlDevice->fillRectangle(colorf::BLACK + 255, x(), y(), texW, texH);
                    }

                    drawMmapTexture();
                    g_sdlDevice->drawTexture(texPtr, x(), y());
                }

                m_buttonAlpha.draw();
                m_buttonExtend.draw();
                return;
            }
        case MMAP_FULLSCREEN:
            {
                drawMmapTexture();
                return;
            }
        default:
            {
                throw bad_reach();
            }
    }
}

bool MMapBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        m_buttonAlpha .setOff();
        m_buttonExtend.setOff();
        return false;
    }

    bool took = false;
    took |= m_buttonAlpha .processEvent(event, valid && !took);
    took |= m_buttonExtend.processEvent(event, valid && !took);
    return took;
}

void MMapBoard::setState(MMapState state)
{
    m_state = state;
    switch(m_state){
        case MMAP_ON:
            {
                setLoc();
                m_buttonAlpha .setOff();
                m_buttonExtend.setOff();
                return;
            }
        case MMAP_EXTENDED:
            {
                setLoc();
                m_buttonAlpha .setOff();
                m_buttonExtend.setOff();
                return;
            }
        default:
            {
                return;
            }
    }
}

void MMapBoard::setLoc()
{
    switch(m_state){
        case MMAP_ON:
            {
                const int rdrW = g_sdlDevice->getRendererWidth();
                const int texW = SDLDeviceHelper::getTextureWidth(g_progUseDB->Retrieve(0X09000000));
                moveTo(rdrW - texW, 0);

                m_buttonAlpha .moveTo( 90, 109);
                m_buttonExtend.moveTo(109, 109);
                return;
            }
        case MMAP_EXTENDED:
            {
                const int rdrW = g_sdlDevice->getRendererWidth();
                const int texW = SDLDeviceHelper::getTextureWidth(g_progUseDB->Retrieve(0X09000001));
                moveTo(rdrW - texW, 0);

                m_buttonAlpha .moveTo(218, 237);
                m_buttonExtend.moveTo(237, 237);
                return;
            }
        default:
            {
                return;
            }
    }
}

void MMapBoard::drawMmapTexture() const
{
    const auto [mapID, mapW, mapH] = m_processRun->getMap();
    const auto [xc, yc] = m_processRun->getMyHero()->location();

    auto [dstX, dstY, dstW, dstH] = [this]() -> std::array<int, 4>
    {
        switch(m_state){
            case MMAP_ON:
            case MMAP_EXTENDED:
                {
                    if(auto texPtr = g_progUseDB->Retrieve(m_state == MMAP_ON ? 0X09000000 : 0X09000001); texPtr){
                        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                        return {x(), y(), texW, texH};
                    }
                    throw fflerror("invalid mmap texture");
                }
            case MMAP_FULLSCREEN:
                {
                    const auto [rdrW, rdrH] = g_sdlDevice->getRendererSize();
                    return {0, 0, rdrW, rdrH};
                }
            default:
                {
                    throw bad_reach();
                }
        }
    }();

    SDL_Texture *texPtr = g_progUseDB->Retrieve(DBCOM_MAPRECORD(mapID).mmapID);
    if(!texPtr){
        return;
    }

    if(m_state == MMAP_FULLSCREEN){
        g_sdlDevice->fillRectangle(colorf::BLACK + 255, dstX, dstY, dstW, dstH);
    }

    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
    const int pixelCenterX = (int)(std::lround((xc * 1.0 / mapW) * texW));
    const int pixelCenterY = (int)(std::lround((yc * 1.0 / mapH) * texH));

    int srcX = pixelCenterX - dstW / 2;
    int srcY = pixelCenterY - dstH / 2;
    int srcW = dstW;
    int srcH = dstH;

    if(srcX < 0){
        dstX -= srcX;
        srcW += srcX;
        srcX  = 0;
    }

    if(srcY < 0){
        dstY -= srcY;
        srcH += srcY;
        srcY  = 0;
    }

    srcW = std::min<int>(srcW, texW - srcX);
    srcH = std::min<int>(srcH, texH - srcY);
    {
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, [this]() -> uint32_t
        {
            switch(m_state){
                case MMAP_ON:
                case MMAP_EXTENDED:
                    {
                        return colorf::WHITE + (m_alphaOn ? 128 : 255);
                    }
                default:
                    {
                        return colorf::WHITE + 255;
                    }
            }
        }());
        g_sdlDevice->drawTexture(texPtr, dstX, dstY, srcX, srcY, srcW, srcH);
    }
}

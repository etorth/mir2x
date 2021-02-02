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
              if(getMmapTexture()){
                  flipExtended();
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
    show(false);
}

void MMapBoard::drawEx(int, int, int, int, int, int) const
{
    drawMmapTexture();
    drawFrame();

    m_buttonAlpha.draw();
    m_buttonExtend.draw();
}

bool MMapBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!show()){
        return false;
    }

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

void MMapBoard::flipExtended()
{
    m_extended = !m_extended;
    setLoc();
    m_buttonAlpha .setOff();
    m_buttonExtend.setOff();
}

void MMapBoard::setLoc()
{
    const int size = getFrameSize();
    m_w = size;
    m_h = size;

    moveTo(g_sdlDevice->getRendererWidth() - w(), 0);

    const int buttonW = std::max<int>(m_buttonAlpha.w(), m_buttonExtend.w());
    const int buttonH = std::max<int>(m_buttonAlpha.h(), m_buttonExtend.h());

    m_buttonAlpha .moveTo(w() - 2 * buttonW, h() - buttonH);
    m_buttonExtend.moveTo(w() -     buttonW, h() - buttonH);
}

void MMapBoard::drawMmapTexture() const
{
    auto texPtr = getMmapTexture();
    if(!texPtr){
        return;
    }

    const auto [mapID, mapW, mapH] = m_processRun->getMap();
    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
    const auto fnGetMmapPLoc = [mapW, mapH, texW, texH](const std::tuple<int, int> &loc) -> std::tuple<int, int>
    {
        return
        {
            (int)(std::lround((std::get<0>(loc) * 1.0 / mapW) * texW)),
            (int)(std::lround((std::get<1>(loc) * 1.0 / mapH) * texH)),
        };
    };

    const auto [heroMmapPX, heroMmapPY] = fnGetMmapPLoc(m_processRun->getMyHero()->location());
    const int srcX = std::min<int>(std::max<int>(0, heroMmapPX - w() / 2), texW - w() / 2);
    const int srcY = std::min<int>(std::max<int>(0, heroMmapPY - h() / 2), texH - h() / 2);
    {
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::WHITE + (m_alphaOn ? 200 : 255));
        g_sdlDevice->drawTexture(texPtr, x(), y(), srcX, srcY, w(), h());
    }

    g_sdlDevice->fillRectangle(colorf::RED + 255, x() + (heroMmapPX - srcX) - 2, y() + (heroMmapPY - srcY) - 2, 5, 5);
    for(const auto &p: m_processRun->getCOList()){
        const auto [coMmapPX, coMmapPY] = fnGetMmapPLoc(p.second->location());
        const auto [color, r] = [this](uint64_t uid) -> std::tuple<uint32_t, int>
        {
            switch(uidf::getUIDType(uid)){
                case UID_PLY:
                    {
                        if(uid == m_processRun->getMyHeroUID()){
                            return {colorf::RED + 255, 2};
                        }
                        else{
                            return {colorf::RED + 255, 1};
                        }
                    }
                case UID_NPC:
                    {
                        return {colorf::YELLOW + 255, 1};
                    }
                case UID_MON:
                    {
                        return {colorf::BLUE + 255, 1};
                    }
                default:
                    {
                        return {0, 0};
                    }
            }
        }(p.first);
        g_sdlDevice->fillRectangle(color, x() + (coMmapPX - srcX) - r, y() + (coMmapPY - srcY) - r, 2 * r + 1, 2 * r + 1);
    }
}

void MMapBoard::drawFrame() const
{
    g_sdlDevice->drawRectangle(colorf::RGBA(60, 60, 60, 255), x(), y(), w(), h());
    if(auto texPtr = g_progUseDB->Retrieve(0X09000006); texPtr){
        g_sdlDevice->drawTexture(texPtr, x(), y());
    }

    if(auto texPtr = g_progUseDB->Retrieve(0X09000007); texPtr){
        g_sdlDevice->drawTexture(texPtr, x() + w() - SDLDeviceHelper::getTextureWidth(texPtr), y());
    }

    if(auto texPtr = g_progUseDB->Retrieve(0X09000008); texPtr){
        g_sdlDevice->drawTexture(texPtr, x(), y() + h() - SDLDeviceHelper::getTextureHeight(texPtr));
    }
}

int MMapBoard::getFrameSize() const
{
    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(getMmapTexture());
    if(m_extended){
        return std::min<int>({texW, texH, 256});
    }else{
        return std::min<int>({texW, texH, 128});
    }
}

SDL_Texture *MMapBoard::getMmapTexture() const
{
    [[maybe_unused]] const auto [mapID, mapW, mapH] = m_processRun->getMap();
    if(const auto mmapID = DBCOM_MAPRECORD(mapID).mmapID){
        return g_progUseDB->Retrieve(mmapID);
    }
    return nullptr;
}

void MMapBoard::flipMmapShow()
{
    flipShow(this);
    setLoc();
}

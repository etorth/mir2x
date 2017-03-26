/*
 * =====================================================================================
 *
 *       Filename: clientmap.cpp
 *        Created: 06/02/2016 14:10:46
 *  Last Modified: 03/26/2017 02:30:40
 *
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

#include "sysconst.hpp"
#include "clientmap.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"

bool ClientMap::Load(uint32_t nMapID)
{
    m_MapID = nMapID;
    if(auto pMapName = SYS_MAPNAME(nMapID)){
        return m_Mir2xMapData.Load(pMapName) ? false : true;
    }
    return false;
}

void ClientMap::Draw(int nSrcX, int nSrcY, int nSrcW, int nSrcH, int nDstX, int nDstY)
{
    int nX0 = (nSrcX - 2 * SYS_MAPGRIDXP - SYS_OBJMAXW) / SYS_MAPGRIDXP;
    int nY0 = (nSrcY - 2 * SYS_MAPGRIDYP - SYS_OBJMAXH) / SYS_MAPGRIDYP;
    int nX1 = (nSrcX + 2 * SYS_MAPGRIDXP + SYS_OBJMAXW + nSrcW) / SYS_MAPGRIDXP;
    int nY1 = (nSrcY + 2 * SYS_MAPGRIDYP + SYS_OBJMAXH + nSrcH) / SYS_MAPGRIDYP;

    int nViewX = nSrcX - nDstX;
    int nViewY = nSrcY - nDstY;

    extern PNGTexDBN *g_PNGTexDBN;
    extern SDLDevice *g_SDLDevice;

    // tiles
    for(int nY = nY0; nY <= nY1; ++nY){
        for(int nX = nX0; nX <= nX1; ++nX){
            if(ValidC(nX, nY) && !(nX % 2) && !(nY % 2)){
                auto nParam = m_Mir2xMapData.Tile(nX, nY).Param;
                if(nParam & 0X80000000){
                    if(auto pTexture = g_PNGTexDBN->Retrieve(nParam& 0X00FFFFFF)){
                        g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - nViewX, nY * SYS_MAPGRIDYP - nViewY);
                    }
                }
            }
        }
    }

    // ground objects
    for(int nY = nY0; nY <= nY1; ++nY){
        for(int nX = nX0; nX <= nX1; ++nX){
            if(ValidC(nX, nY) && (m_Mir2xMapData.Cell(nX, nY).Param & 0X80000000)){
                // for obj-0
                {
                    auto nParam = m_Mir2xMapData.Cell(nX, nY).Obj[0].Param;
                    if(nParam & 0X80000000){
                        auto nObjParam = m_Mir2xMapData.Cell(nX, nY).ObjParam;
                        if(nObjParam & ((uint32_t)(1) << 22)){
                            if(auto pTexture = g_PNGTexDBN->Retrieve(nParam & 0X00FFFFFF)){
                                int nH = 0;
                                if(!SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH)){
                                    g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - nViewX, (nY + 1) * SYS_MAPGRIDYP - nViewY - nH);
                                }
                            }
                        }
                    }
                }

                // for obj-1
                {
                    auto nParam = m_Mir2xMapData.Cell(nX, nY).Obj[0].Param;
                    if(nParam & 0X80000000){
                        auto nObjParam = m_Mir2xMapData.Cell(nX, nY).ObjParam;
                        if(nObjParam & ((uint32_t)(1) << 6)){
                            if(auto pTexture = g_PNGTexDBN->Retrieve(nParam & 0X00FFFFFF)){
                                int nH = 0;
                                if(!SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH)){
                                    g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - nViewX, (nY + 1) * SYS_MAPGRIDYP - nViewY - nH);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // over-ground objects
    for(int nY = nY0; nY <= nY1; ++nY){
        for(int nX = nX0; nX <= nX1; ++nX){
            if(ValidC(nX, nY) && (m_Mir2xMapData.Cell(nX, nY).Param & 0X80000000)){
                // for obj-0
                {
                    auto nParam = m_Mir2xMapData.Cell(nX, nY).Obj[0].Param;
                    if(nParam & 0X80000000){
                        auto nObjParam = m_Mir2xMapData.Cell(nX, nY).ObjParam;
                        if(!(nObjParam & ((uint32_t)(1) << 22))){
                            if(auto pTexture = g_PNGTexDBN->Retrieve(nParam & 0X00FFFFFF)){
                                int nH = 0;
                                if(!SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH)){
                                    g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - nViewX, (nY + 1) * SYS_MAPGRIDYP - nViewY - nH);
                                }
                            }
                        }
                    }
                }

                // for obj-1
                {
                    auto nParam = m_Mir2xMapData.Cell(nX, nY).Obj[0].Param;
                    if(nParam & 0X80000000){
                        auto nObjParam = m_Mir2xMapData.Cell(nX, nY).ObjParam;
                        if(!(nObjParam & ((uint32_t)(1) << 6))){
                            if(auto pTexture = g_PNGTexDBN->Retrieve(nParam & 0X00FFFFFF)){
                                int nH = 0;
                                if(!SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH)){
                                    g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - nViewX, (nY + 1) * SYS_MAPGRIDYP - nViewY - nH);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // others
}

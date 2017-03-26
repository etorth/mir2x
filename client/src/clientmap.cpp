/*
 * =====================================================================================
 *
 *       Filename: clientmap.cpp
 *        Created: 06/02/2016 14:10:46
 *  Last Modified: 03/25/2017 14:40:34
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

bool ClientMap::Load(uint32_t nMapID)
{
    m_MapID = nMapID;
    if(auto pMapName = SYS_MAPNAME(nMapID)){
        return m_Mir2xMapData.Load(pMapName) ? false : true;
    }
    return false;
}

void ClientMap::Draw(int nViewX, int nViewY, int nViewW, int nViewH,    // view region
        int nMaxObjW, int nMaxObjH,                                     // operation addition margin
        const std::function<void(int, int, uint32_t)> &fnDrawTile,      //
        const std::function<void(int, int, uint32_t)> &fnDrawObj,       //
        const std::function<void(int, int)> &fnDrawActor,               //
        const std::function<void(int, int)> &fnDrawExt)                 //
{
    // to make it safe
    int nCellX0 = (nViewX - 2 * SYS_MAPGRIDXP - nMaxObjW         ) / SYS_MAPGRIDXP;
    int nCellY0 = (nViewY - 2 * SYS_MAPGRIDYP - nMaxObjH         ) / SYS_MAPGRIDYP;
    int nCellX1 = (nViewX + 2 * SYS_MAPGRIDXP + nMaxObjW + nViewW) / SYS_MAPGRIDXP;
    int nCellY1 = (nViewY + 2 * SYS_MAPGRIDYP + nMaxObjH + nViewH) / SYS_MAPGRIDYP;

    // 1. draw tile, this should be done seperately
    for(int nY = nCellY0; nY <= nCellY1; ++nY){
        for(int nX = nCellX0; nX <= nCellX1; ++nX){
            if(ValidC(nX, ))
            // 1. boundary check
            if(!ValidC(nX, nY)){ continue; }

            // 2. for tile
            if(!(nY % 2) && !(nX % 2) && TileValid(nX, nY)){
                fnDrawTile(nX, nY, Tile(nX, nY));
            }
        }
    }

    // 2. draw everything on ground
    for(int nY = nCellY0; nY <= nCellY1; ++nY){
        for(int nX = nCellX0; nX <= nCellX1; ++nX){
            // 1. validate the cell, draw overgournd objects here
            if(ValidC(nX, nY)){
                // 1-1. draw ground cell object
                for(int nIndex = 0; nIndex < 2; ++nIndex){
                    if(GroundObjectValid(nX, nY, nIndex)){
                        fnDrawObj(nX, nY, Object(nX, nY, nIndex));
                    }
                }

                // 1-2. draw actor
                // fnDrawActor(nX, nY);

                // 1-3. draw over ground cell object
                for(int nIndex = 0; nIndex < 2; ++nIndex){
                    if(!GroundObjectValid(nX, nY, nIndex)){
                        fnDrawObj(nX, nY, Object(nX, nY, nIndex));
                    }
                }

                // tricky part for mir2 map data
                // we have to draw actor after all map objects
                fnDrawActor(nX, nY);
            }

            // 2. draw ext, even the cell is not valid we need to draw it
            fnDrawExt(nX, nY);

        }
    }
}

/*
 * =====================================================================================
 *
 *       Filename: clientmap.cpp
 *        Created: 06/02/2016 14:10:46
 *  Last Modified: 06/02/2016 14:13:07
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
    auto pMapName = SYS_MAPNAME(nMapID);
    return pMapName ? m_Mir2xMap.Load(pMapName) : false;
}

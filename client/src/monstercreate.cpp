/*
 * =====================================================================================
 *
 *       Filename: monstercreate.cpp
 *        Created: 04/08/2017 17:00:20
 *  Last Modified: 04/09/2017 01:36:41
 *
 *    Description: factory model for all monster
 *                 make this file stand alone since it will include all monster types
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
#include "monster.hpp"
#include "zumamonster.hpp"
#include "protocoldef.hpp"

Monster *Monster::Create(uint32_t nUID, uint32_t nMonsterID, ProcessRun *pRun, const ActionNode &rstAction)
{
    switch(nMonsterID){
        case MONSTERID_ZUMA0:
        case MONSTERID_ZUMA1:
        case MONSTERID_ZUMA2:
            {
                return ZumaMonster::Create(nUID, nMonsterID, pRun, rstAction);
            }
        default:
            {
                auto pNew = new Monster(nUID, nMonsterID, pRun);
                pNew->m_CurrMotion.Motion    = MOTION_STAND;
                pNew->m_CurrMotion.Speed     = 0;
                pNew->m_CurrMotion.Direction = DIR_UP;
                pNew->m_CurrMotion.X         = rstAction.X;
                pNew->m_CurrMotion.Y         = rstAction.Y;
                pNew->m_CurrMotion.EndX      = rstAction.EndX;
                pNew->m_CurrMotion.EndY      = rstAction.EndY;

                if(pNew->ParseNewAction(rstAction)){
                    return pNew;
                }

                delete pNew;
                return nullptr;
            }
    }
}

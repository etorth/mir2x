/*
 * =====================================================================================
 *
 *       Filename: monstercreate.cpp
 *        Created: 04/08/2017 17:00:20
 *  Last Modified: 04/09/2017 00:40:46
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
        case MONSTERID_DEER:
            {
                return Monster::Create(nUID, nMonsterID, pRun, rstAction);
            }
        default:
            {
                return nullptr;
            }
    }
}

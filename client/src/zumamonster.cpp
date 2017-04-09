/*
 * =====================================================================================
 *
 *       Filename: zumamonster.cpp
 *        Created: 04/08/2017 16:34:50
 *  Last Modified: 04/09/2017 00:49:04
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
#include "zumamonster.hpp"

ZumaMonster *ZumaMonster::Create(uint32_t nUID, uint32_t nMonsterID, ProcessRun *pRun, const ActionNode &rstAction)
{
    switch(nMonsterID){
        case MONSTERID_ZUMA0:
        case MONSTERID_ZUMA1:
        case MONSTERID_ZUMA2:
            {
                auto pNew = new ZumaMonster(nUID, nMonsterID, pRun);
                switch(rstAction.Action){
                    case ACTION_STAND:
                        {
                            if(rstAction.ActionParam){
                                pNew->m_CurrMotion.Motion = MOTION_PETRIFIED;
                                pNew->m_CurrMotion.X      = rstAction.X;
                                pNew->m_CurrMotion.Y      = rstAction.Y;
                                pNew->m_CurrMotion.EndX   = rstAction.X;
                                pNew->m_CurrMotion.EndY   = rstAction.Y;
                            }else{
                                pNew->m_CurrMotion.Motion = MOTION_STAND;
                                pNew->m_CurrMotion.X      = rstAction.X;
                                pNew->m_CurrMotion.Y      = rstAction.Y;
                                pNew->m_CurrMotion.EndX   = rstAction.X;
                                pNew->m_CurrMotion.EndY   = rstAction.Y;
                            }
                            break;
                        }
                    default:
                        {
                            pNew->m_CurrMotion.Motion = MOTION_STAND;
                            pNew->m_CurrMotion.X      = rstAction.X;
                            pNew->m_CurrMotion.Y      = rstAction.Y;
                            pNew->m_CurrMotion.EndX   = rstAction.X;
                            pNew->m_CurrMotion.EndY   = rstAction.Y;
                            break;
                        }
                }

                if(pNew->ParseNewAction(rstAction)){
                    return pNew;
                }

                delete pNew;
                return nullptr;
            }
        default:
            {
                return nullptr;
            }
    }
}

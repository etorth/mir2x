/*
 * =====================================================================================
 *
 *       Filename: clientpathfinder.cpp
 *        Created: 03/28/2017 21:15:25
 *  Last Modified: 03/29/2017 00:56:21
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

#include "game.hpp"
#include "processrun.hpp"
#include "clientpathfinder.hpp"

ClientPathFinder::ClientPathFinder(bool bCheckCreature)
    : AStarPathFinder([bCheckCreature](int nSrcX, int nSrcY, int nDstX, int nDstY) -> bool {
            extern Game *g_Game;
            if(auto pRun = g_Game->ProcessValid(PROCESSID_RUN)){
                return ((ProcessRun *)(pRun))->CanMove(bCheckCreature, nSrcX, nSrcY, nDstX, nDstY);
            }
            return false;
      })
    , m_CheckCreature(bCheckCreature)
{}

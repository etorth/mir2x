/*
 * =====================================================================================
 *
 *       Filename: clientpathfinder.cpp
 *        Created: 03/28/2017 21:15:25
 *  Last Modified: 07/04/2017 13:11:57
 *
 *    Description: class for path finding in ProcessRun
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

#include "log.hpp"
#include "game.hpp"
#include "processrun.hpp"
#include "clientpathfinder.hpp"

ClientPathFinder::ClientPathFinder(bool bCheckGround, bool bCheckCreature, int nMaxStep)
    : AStarPathFinder(
            [bCheckGround, nMaxStep](int nSrcX, int nSrcY, int nDstX, int nDstY) -> bool
            {
                // for client we can disable bCheckGround
                // which allows invalid grid but assign a very high value to it
                // then stop heros if they're trying to cross those invalid grids

                // this helps if player clicks on invalid grid
                // we should still give an path to make it move to the right direction
                //
                // this will spend much more time but ok for client

                // here I didn't check distance between (nSrcX, nSrcY) -> (nDstX, nDstY)
                // in AStarPathFinder::GetSuccessor() I only add valid (nDstX, nDstY) inside proper distance
                // so I can skip the checking here if bCheckGround is set

                if(0){
                    if(true
                            && nMaxStep != 1
                            && nMaxStep != 2
                            && nMaxStep != 3){

                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", nMaxStep);
                        return false;
                    }

                    int nDistance2 = LDistance2(nSrcX, nSrcY, nDstX, nDstY);
                    if(true
                            && nDistance2 != 1
                            && nDistance2 != 2
                            && nDistance2 != nMaxStep * nMaxStep
                            && nDistance2 != nMaxStep * nMaxStep * 2){

                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_FATAL, "Invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
                        return false;
                    }
                }

                if(bCheckGround){
                    extern Game *g_Game;
                    if(auto pRun = g_Game->ProcessValid(PROCESSID_RUN)){
                        // won't check bCheckCreature
                        // this class won't fail if creatures blocking paths
                        // instead we put a high cost for those grid occupied by creatures
                        return ((ProcessRun *)(pRun))->CanMove(false, nSrcX, nSrcY, nDstX, nDstY);
                    }else{
                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_FATAL, "Current process is not ProcessRun");
                        return false;
                    }
                }else{ return true; }
            },

            [bCheckCreature, nMaxStep](int nSrcX, int nSrcY, int nDstX, int nDstY) -> double
            {
                // no matter we check ground or not
                // we assign very high value to invalid grids

                // as metioned above
                // I can skip to check distance between (nSrcX, nSrcY, nDstX, nDstY)
                // I can skip to check nMaxStep = 1, 2, 3
                if(0){
                    if(true
                            && nMaxStep != 1
                            && nMaxStep != 2
                            && nMaxStep != 3){

                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", nMaxStep);
                        return 10000.00;
                    }

                    int nDistance2 = LDistance2(nSrcX, nSrcY, nDstX, nDstY);
                    if(true
                            && nDistance2 != 1
                            && nDistance2 != 2
                            && nDistance2 != nMaxStep * nMaxStep
                            && nDistance2 != nMaxStep * nMaxStep * 2){

                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_FATAL, "Invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
                        return 10000.00;
                    }
                }

                // calculate cost of current hop
                // 1. if bCheckCreature set then return high cost if creatures on the way
                // 2. if (nDstX, nDstY) is not reach-able, return high value instead of refuse it
                // 3. add extra weight to prefer go straight, otherwise i.e.
                //          A B C D
                //          E F G H
                //    path (A->F->C) and (A->B->C) are of same cost
                //    and so the algorithm can't prefer (A->B->C) rather than (A->F->C)
                //
                // 4. give very close weight for StepSize = 1 and StepSize = MaxStep to prefer bigger hops
                //    but I have to make Weight(MaxStep) = Weight(1) + dW ( > 0 ), reason:
                //          for MaxStep = 3 and path as following:
                //                          A B C D E
                //          if I want to move (A->C), we can do (A->B->C) and (A->D->C)
                //          then if there are of same weight I can't prefer (A->B->C)
                //
                //          but for MaxStep = 2 I don't have this issue
                //    actually for dW < Weight(1) is good enough

                // cost :   valid  :     1.00
                //        occupied :   100.00
                //         invalid : 10000.00
                //
                // we should have cost(invalid) >> cost(occupied), otherwise
                //          XXAXX
                //          XOXXX
                //          XXBXX
                // path (A->O->B) and (A->X->B) are of equal cost

                double fExtraPen = 0.00;
                switch(LDistance2(nSrcX, nSrcY, nDstX, nDstY)){
                    case  1: fExtraPen = 0.00 + 0.01; break;
                    case  2: fExtraPen = 0.10 + 0.01; break;
                    case  4: fExtraPen = 0.00 + 0.02; break;
                    case  8: fExtraPen = 0.10 + 0.02; break;
                    case  9: fExtraPen = 0.00 + 0.03; break;
                    case 18: fExtraPen = 0.10 + 0.03; break;
                    default:
                        {
                            extern Log *g_Log;
                            g_Log->AddLog(LOGTYPE_FATAL, "Invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
                            return 10000.00;
                        }
                }

                extern Game *g_Game;
                if(auto pRun = (ProcessRun *)(g_Game->ProcessValid(PROCESSID_RUN))){
                    if(pRun->CanMove(false, nDstX, nDstY)){
                        if(bCheckCreature){
                            // if there is no creatures on the way we take it
                            // however if there is, we can still take it but with very high cost
                            return (pRun->CanMove(true, nDstX, nDstY) ? 1.00 : 100.00) + fExtraPen;
                        }else{
                            // won't check creature
                            // then all walk-able step get cost 1.0 + delta
                            return 1.00 + fExtraPen;
                        }
                    }else{
                        // can't go through, return the infinite
                        // be careful of following situation which could make mistake
                        //
                        //     XOOAOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
                        //     XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXO
                        //     XOOBOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
                        //
                        // here ``O" means ``can pass" and ``X" means not, then if we do move (A->B)
                        // if the path is too long then likely it takes(A->X->B) rather than (A->OOOOOOO...OOO->B)
                        //
                        // method to solve it:
                        //  1. put path length constraits
                        //  2. define inifinite = Map::W() * Map::H() as any path can have
                        return 10000.00;
                    }
                }else{
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_FATAL, "Current process is not ProcessRun");
                    return 10000.00;
                }
            },

            // max step length for each hop, valid step size : 1, 2, 3
            // check value in AStarPathFinder::AStarPathFinder() and ClientPathFinder::ClientPathFinder()
            nMaxStep
      )
{
    // we do it here to complete the logic
    // this also will be checked in AStarPathFinder::AStarPathFinder()
    switch(nMaxStep){
        case 1:
        case 2:
        case 3:
            {
                break;
            }
        default:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", nMaxStep);
                break;
            }
    }
}

/*
 * =====================================================================================
 *
 *       Filename: clientpathfinder.cpp
 *        Created: 03/28/2017 21:15:25
 *  Last Modified: 04/17/2017 11:54:23
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

#include "log.hpp"
#include "game.hpp"
#include "processrun.hpp"
#include "clientpathfinder.hpp"

ClientPathFinder::ClientPathFinder(bool bCheckCreature)
    : AStarPathFinder(
            // 1. step check function
            [bCheckCreature](int nSrcX, int nSrcY, int nDstX, int nDstY) -> bool {
                extern Game *g_Game;
                if(auto pRun = g_Game->ProcessValid(PROCESSID_RUN)){
                    return ((ProcessRun *)(pRun))->CanMove(bCheckCreature, nSrcX, nSrcY, nDstX, nDstY);
                }
                return false;
            },

            // 2. move cost function, for directions as following
            //    
            //                  0
            //               7     1
            //             6    x    2
            //               5     3
            //                  4
            //    we put directions 1, 3, 5, 7 as higher cost because for following grids:
            //
            //              A B C D E
            //              F G H I J
            //              K L M N O
            //              P Q R S T
            //
            //    if we make all directions equally cost, then G->I: G -> C -> I
            //    we want the result as G -> H -> I
            [bCheckCreature](int nSrcX, int nSrcY, int nDstX, int nDstY) -> double {
                extern Game *g_Game;
                if(auto pRun = g_Game->ProcessValid(PROCESSID_RUN)){
                    if(((ProcessRun *)(pRun))->CanMove(bCheckCreature, nSrcX, nSrcY, nDstX, nDstY)){
                        static const int nDirV[3][3] = {
                            {7, 0, 1},
                            {6, 0, 2},
                            {5, 4, 3},
                        };

                        int nSDX = (nDstX > nSrcX) - (nDstX < nSrcX) + 1;
                        int nSDY = (nDstY > nSrcY) - (nDstY < nSrcY) + 1;

                        return (nDirV[nSDY][nSDX] % 2) ? 1.1 : 1.0;
                    }
                }

                // else errors happens
                {
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_FATAL, "fatal error in path finding");
                    return 10000.0;
                }
            }
      )
    , m_CheckCreature(bCheckCreature)
{}

/*
 * =====================================================================================
 *
 *       Filename: pathfinder.cpp
 *        Created: 03/29/2017 00:59:29
 *  Last Modified: 01/14/2018 18:54:09
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
#include "mathfunc.hpp"
#include "pathfinder.hpp"

bool AStarPathFinder::Search(int nX0, int nY0, int nX1, int nY1)
{
    AStarPathFinderNode stNode0 {nX0, nY0, -1, this};
    AStarPathFinderNode stNode1 {nX1, nY1, -1, this};
    SetStartAndGoalStates(stNode0, stNode1);

    unsigned int nSearchState;
    do{
        nSearchState = SearchStep();
    }while(nSearchState == AStarSearch<AStarPathFinderNode>::SEARCH_STATE_SEARCHING);
    return nSearchState == AStarSearch<AStarPathFinderNode>::SEARCH_STATE_SUCCEEDED;
}

int PathFind::MaxReachNode(const PathFind::PathNode *pNodeV, size_t nSize, size_t nMaxStepLen)
{
    if(true
            && pNodeV           //
            && nSize            // if nSize == 1 then return 0
            && nMaxStepLen){    // doesn't request nMaxStepLen < nSize here

        // 1. verify all nodes
        for(size_t nIndex = 1; nIndex < nSize; ++nIndex){
            switch(LDistance2(pNodeV[nIndex].X, pNodeV[nIndex].Y, pNodeV[nIndex - 1].X, pNodeV[nIndex - 1].Y)){
                case 1:
                case 2:
                    {
                        break;
                    }
                default:
                    {
                        return -1;
                    }
            }
        }

        // 2. calculate the max reach node
        switch(nSize){
            case 0  : return -1;
            case 1  : return  0;
            case 2  : return  1;
            default : {
                          if(nMaxStepLen < nSize){
                              int nDX = std::abs<int>(pNodeV[nMaxStepLen].X - pNodeV[0].X);
                              int nDY = std::abs<int>(pNodeV[nMaxStepLen].Y - pNodeV[0].Y);
                              if(true
                                      && (std::max<size_t>(nDX, nDY) == nMaxStepLen)
                                      && (std::min<size_t>(nDX, nDY) == 0 || nDX == nDY)){
                                  return (int)(nMaxStepLen);
                              }
                          }

                          // if 1. no enough nodes
                          //    2. nodes can't reach nMaxStepLen straight
                          // then force use single step walk
                          return 1;
                      }
        }
    }
    return -1;
}

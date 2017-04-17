/*
 * =====================================================================================
 *
 *       Filename: pathfinder.cpp
 *        Created: 03/29/2017 00:59:29
 *  Last Modified: 04/17/2017 00:11:23
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
#include "pathfinder.hpp"

bool AStarPathFinder::Search(int nX0, int nY0, int nX1, int nY1)
{
    m_FindPathDstX = nX1;
    m_FindPathDstY = nY1;

    AStarPathFinderNode stNode0 {nX0, nY0, this};
    AStarPathFinderNode stNode1 {nX1, nY1, this};
    SetStartAndGoalStates(stNode0, stNode1);

    unsigned int nSearchState;
    do{
        nSearchState = SearchStep();
    }while(nSearchState == AStarSearch<AStarPathFinderNode>::SEARCH_STATE_SEARCHING);
    return nSearchState == AStarSearch<AStarPathFinderNode>::SEARCH_STATE_SUCCEEDED;
}

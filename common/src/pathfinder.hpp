/*
 * =====================================================================================
 *
 *       Filename: pathfinder.hpp
 *        Created: 03/28/2017 17:04:54
 *  Last Modified: 04/17/2017 11:45:15
 *
 *    Description: A-Star algorithm for path find
 *
 *                 previously I put a (dstX, dstY) in AStarPathFinder to make each
 *                 time when search next node, it prefers the one with the right
 *                 direction, but this seems won't help
 *
 *                 if we have a prefperence, we should make it by the cost function,
 *                 rather than hack the source to make a priority
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

#pragma once

#include <functional>

#include "fsa.h"
#include "stlastar.h"

class AStarPathFinderNode;
class AStarPathFinder: public AStarSearch<AStarPathFinderNode>
{
    public:
        friend class AStarPathFinderNode;

    private:
        std::function<bool  (int, int, int, int)> m_MoveChecker;
        std::function<double(int, int, int, int)> m_MoveCost;

    public:
        AStarPathFinder(std::function<bool(int, int, int, int)> fnMoveChecker)
            : AStarSearch<AStarPathFinderNode>()
            , m_MoveChecker(fnMoveChecker)
            , m_MoveCost()
        {
            assert(fnMoveChecker);
        }

        AStarPathFinder(std::function<bool(int, int, int, int)> fnMoveChecker, std::function<double(int, int, int, int)> fnMoveCost)
            : AStarSearch<AStarPathFinderNode>()
            , m_MoveChecker(fnMoveChecker)
            , m_MoveCost(fnMoveCost)
        {
            assert(fnMoveChecker);
        }

        ~AStarPathFinder()
        {
            FreeSolutionNodes();
            EnsureMemoryFreed();
        }

    public:
        bool Search(int, int, int, int);
};

class AStarPathFinderNode
{
    public:
        friend class AStarPathFinder;
        friend class AStarSearch<AStarPathFinderNode>;

    private:
        int m_X;
        int m_Y;

    private:
        AStarPathFinder *m_Finder;

    private:
        AStarPathFinderNode() = default;
        AStarPathFinderNode(int nX, int nY, AStarPathFinder *pFinder)
            : m_X(nX)
            , m_Y(nY)
            , m_Finder(pFinder)
        {
            assert(pFinder);
        }

    public:
       ~AStarPathFinderNode() = default;

    public:
       int X(){ return m_X; }
       int Y(){ return m_Y; }

    public:
        float GoalDistanceEstimate(AStarPathFinderNode &rstGoalNode)
        {
            // we use Chebyshev's distance instead of Manhattan distance
            return std::max<float>(std::abs(rstGoalNode.m_X - m_X), std::abs(rstGoalNode.m_Y - m_Y));
        }

        bool IsGoal(AStarPathFinderNode &rstGoalNode)
        {
            return (m_X == rstGoalNode.m_X) && (m_Y == rstGoalNode.m_Y);
        }

        bool GetSuccessors(AStarSearch<AStarPathFinderNode> *pAStarSearch, AStarPathFinderNode *pParentNode)
        {
            static const int nDX[] = { 0, +1, +1, +1,  0, -1, -1, -1};
            static const int nDY[] = {-1, -1,  0, +1, +1, +1,  0, -1};

            for(int nIndex = 0; nIndex < 8; ++nIndex){
                int nNewX = m_X + nDX[nIndex];
                int nNewY = m_Y + nDY[nIndex];

                if(true
                        && pParentNode
                        && pParentNode->X() == nNewX
                        && pParentNode->Y() == nNewY){ continue; }

                // m_MoveChecker() directly refuse invalid (nX, nY) as false
                // this ensures if pParentNode not null then pParentNode->(X, Y) is always valid
                if(m_Finder->m_MoveChecker(m_X, m_Y, nNewX, nNewY)){
                    AStarPathFinderNode stFinderNode {nNewX, nNewY, m_Finder};
                    pAStarSearch->AddSuccessor(stFinderNode);
                }
            }

            // seems we have to always return true
            // return false means out of memory in the code
            return true;
        }

        float GetCost(AStarPathFinderNode &rstNode)
        {
            if(m_Finder->m_MoveCost){
                auto nCost = m_Finder->m_MoveCost(m_X, m_Y, rstNode.m_X, rstNode.m_Y);
                return (nCost > 0) ? (float)(nCost) : 1.0;
            }else{
                return 1.0;
            }
        }

        bool IsSameState(AStarPathFinderNode &rstNode)
        {
            return (m_X == rstNode.m_X) && (m_Y == rstNode.m_Y);
        }
};

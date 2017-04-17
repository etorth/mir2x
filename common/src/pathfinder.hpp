/*
 * =====================================================================================
 *
 *       Filename: pathfinder.hpp
 *        Created: 03/28/2017 17:04:54
 *  Last Modified: 04/17/2017 01:04:14
 *
 *    Description: A-Star algorithm for path find
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
        std::function<bool(int, int, int, int)> m_MoveChecker;
        std::function<int (int, int, int, int)> m_MoveCost;

    private:
        // actually we can still find the path without these dst info
        // but this dst info can give us a smooth result
        //
        // for each step, we try direction (currX, currY) -> (dstX, dstY) first
        // rather than always up->right->down->left in GetSuccessors()
        int m_FindPathDstX;
        int m_FindPathDstY;

    public:
        AStarPathFinder(std::function<bool(int, int, int, int)> fnMoveChecker)
            : AStarSearch<AStarPathFinderNode>()
            , m_MoveChecker(fnMoveChecker)
            , m_MoveCost()
            , m_FindPathDstX(-1)
            , m_FindPathDstY(-1)
        {
            assert(fnMoveChecker);
        }

        AStarPathFinder(std::function<bool(int, int, int, int)> fnMoveChecker, std::function<int(int, int, int, int)> fnMoveCost)
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
        int m_CurrX;
        int m_CurrY;

    private:
        AStarPathFinder *m_Finder;

    private:
        AStarPathFinderNode() = default;
        AStarPathFinderNode(int nX, int nY, AStarPathFinder *pFinder)
            : m_CurrX(nX)
            , m_CurrY(nY)
            , m_Finder(pFinder)
        {
            assert(pFinder);
        }

    public:
       ~AStarPathFinderNode() = default;

    public:
       int X(){ return m_CurrX; }
       int Y(){ return m_CurrY; }

    public:
        float GoalDistanceEstimate(AStarPathFinderNode &rstGoalNode)
        {
            // we use Chebyshev's distance instead of Manhattan distance
            return std::max<float>(std::abs(rstGoalNode.m_CurrX - m_CurrX), std::abs(rstGoalNode.m_CurrY - m_CurrY));
        }

        bool IsGoal(AStarPathFinderNode &rstGoalNode)
        {
            return (m_CurrX == rstGoalNode.m_CurrX) && (m_CurrY == rstGoalNode.m_CurrY);
        }

        bool GetSuccessors(AStarSearch<AStarPathFinderNode> *pAStarSearch, AStarPathFinderNode *pParentNode)
        {
            // use internal direction index as 0 ~ 7
            //
            //          0
            //       7     1
            //     6    x    2
            //       5     3
            //          4
            //
            // shouldn't have value at x
            // but put it as 0 here to avoid branch in the loop
            static const int nDirectionV[3][3] = {
                {7, 0, 1},
                {6, 0, 2},
                {5, 4, 3},
            };

            // Direction(0 ~ 7) -> (dX, dY)
            //                         0   1   2   3   4   5   6   7
            static const int nDX[] = { 0, +1, +1, +1,  0, -1, -1, -1};
            static const int nDY[] = {-1, -1,  0, +1, +1, +1,  0, -1};

            // with different (currX, currY) -> (dstX, dstY)
            // we put sorted direction list we should try to get smooth result
            static const int nIndexV[8][8] = {
                {0, 1, 7, 2, 6, 3, 5, 4}, // DX = 0, DY < 0, dst at direction 0
                {1, 2, 0, 3, 7, 4, 6, 5}, // DX > 0, DY < 0, dst at direction 1
                {2, 3, 1, 4, 0, 5, 7, 6}, // DX > 0, DY = 0, dst at direction 2
                {3, 4, 2, 5, 1, 6, 0, 7}, //                                  3
                {4, 5, 3, 6, 2, 7, 1, 0}, //                                  4
                {5, 6, 4, 7, 3, 0, 2, 1}, //                                  5
                {6, 7, 5, 0, 4, 1, 3, 2}, //                                  6
                {7, 0, 6, 1, 5, 2, 4, 3}, //                                  7
            };

            int nSDX = (m_Finder->m_FindPathDstX > m_CurrX) - (m_Finder->m_FindPathDstX < m_CurrX) + 1;
            int nSDY = (m_Finder->m_FindPathDstY > m_CurrY) - (m_Finder->m_FindPathDstY < m_CurrY) + 1;

            for(int nIndex = 0; nIndex < 8; ++nIndex){
                int nNewX = m_CurrX + nDX[nIndexV[nDirectionV[nSDY][nSDX]][nIndex]];
                int nNewY = m_CurrY + nDY[nIndexV[nDirectionV[nSDY][nSDX]][nIndex]];

                if(true
                        && pParentNode
                        && pParentNode->X() == nNewX
                        && pParentNode->Y() == nNewY){ continue; }

                // m_MoveChecker() directly refuse invalid (nX, nY) as false
                // this ensures if pParentNode not null then pParentNode->(X, Y) is always valid
                if(m_Finder->m_MoveChecker(m_CurrX, m_CurrY, nNewX, nNewY)){
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
                auto nCost = m_Finder->m_MoveCost(m_CurrX, m_CurrY, rstNode.m_CurrX, rstNode.m_CurrY);
                return (nCost > 0) ? (nCost * 1.0) : 1.0;
            }else{
                return 1.0;
            }
        }

        bool IsSameState(AStarPathFinderNode &rstNode)
        {
            return (m_CurrX == rstNode.m_CurrX) && (m_CurrY == rstNode.m_CurrY);
        }
};

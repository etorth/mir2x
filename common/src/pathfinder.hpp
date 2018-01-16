/*
 * =====================================================================================
 *
 *       Filename: pathfinder.hpp
 *        Created: 03/28/2017 17:04:54
 *  Last Modified: 01/16/2018 15:14:49
 *
 *    Description: A-Star algorithm for path finding
 *
 *                 if we have a prefperence, we should make it by the cost function,
 *                 rather than hack the source to make a priority
 *
 *                 support jump on the map with step size as (1, 2, 3)
 *                 store direction information in each node and calculate cost with it
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
#include <cmath>
#include <functional>

#include "fsa.h"
#include "stlastar.h"
#include "condcheck.hpp"
#include "protocoldef.hpp"

namespace PathFind
{
    struct PathNode final
    {
        int X;
        int Y;

        PathNode(int nX = -1, int nY = -1)
            : X(nX)
            , Y(nY)
        {}

        // I don't want to make it friend
        // which needs another free function declaration
        bool operator == (const PathNode & rstNode)
        {
            return (rstNode.X == X) && (rstNode.Y == Y);
        }
    };

    inline int GetBack(int nDirection)
    {
        switch (nDirection){
            case DIR_UP       : return DIR_DOWN;
            case DIR_DOWN     : return DIR_UP;
            case DIR_LEFT     : return DIR_RIGHT;
            case DIR_RIGHT    : return DIR_LEFT;
            case DIR_UPLEFT   : return DIR_DOWNRIGHT;
            case DIR_UPRIGHT  : return DIR_DOWNLEFT;
            case DIR_DOWNLEFT : return DIR_UPRIGHT;
            case DIR_DOWNRIGHT: return DIR_UPLEFT;
            default           : return DIR_NONE;
        }
    }

    inline bool GetFrontLocation(int *pX, int *pY, int nX, int nY, int nDirection, int nLen = 1)
    {
        static const int nDX[] = { 0, +1, +1, +1,  0, -1, -1, -1};
        static const int nDY[] = {-1, -1,  0, +1, +1, +1,  0, -1};

        if(true
                && nDirection > DIR_NONE
                && nDirection < DIR_MAX){

            if(pX){ *pX = nX + nLen * nDX[nDirection - (DIR_NONE + 1)]; }
            if(pY){ *pY = nY + nLen * nDY[nDirection - (DIR_NONE + 1)]; }

            return true;
        }
        return false;
    }

    inline bool GetBackLocation(int *pX, int *pY, int nX, int nY, int nDirection, int nLen = 1)
    {
        return GetFrontLocation(pX, pY, nX, nY, GetBack(nDirection), nLen);
    }

    // return direction for src -> dst
    // direction code defined in protocoldef.hpp
    inline int GetDirection(int nSrcX, int nSrcY, int nDstX, int nDstY)
    {
        static const int nDirV[][3]
        {
            {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
            {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
            {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT},
        };

        int nDX = (nDstX > nSrcX) - (nDstX < nSrcX);
        int nDY = (nDstY > nSrcY) - (nDstY < nSrcY);

        return nDirV[nDY + 1][nDX + 1];
    }

    int MaxReachNode(const PathFind::PathNode *, size_t, size_t);
}

class AStarPathFinderNode;
class AStarPathFinder: public AStarSearch<AStarPathFinderNode>
{
    private:
        std::function<bool  (int, int, int, int)> m_MoveChecker;
        std::function<double(int, int, int, int)> m_MoveCost;

    private:
        int m_MaxStep;

    public:
        friend class AStarPathFinderNode;

    public:
        AStarPathFinder(
                std::function<  bool(int, int, int, int)> fnMoveChecker, // (x0, y0) -> (x1, y1) : possibility
                std::function<double(int, int, int, int)> fnMoveCost,    // (x0, y0) -> (x1, y1) : cost
                int nMaxStepSize = 1)                                    // (x0, y0) -> (x1, y1) : max step size
            : AStarSearch<AStarPathFinderNode>()
            , m_MoveChecker(fnMoveChecker)
            , m_MoveCost(fnMoveCost)
            , m_MaxStep(nMaxStepSize)
        {
            condcheck(m_MoveChecker);
            condcheck(m_MoveCost);
            condcheck(false
                    || (m_MaxStep == 1)
                    || (m_MaxStep == 2)
                    || (m_MaxStep == 3));
        }

    public:
        ~AStarPathFinder()
        {
            FreeSolutionNodes();
            EnsureMemoryFreed();
        }

    private:
        int MaxStep() const
        {
            return m_MaxStep;
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
        // direction when *stopping* at current place
        // direction map: 0 -> DIR_UP
        //                1 -> DIR_UPRIGHT
        //                2 -> DIR_RIGHT
        //                3 -> DIR_DOWNRIGHT
        //                4 -> DIR_DOWN
        //                5 -> DIR_DOWNLEFT
        //                6 -> DIR_LEFT
        //                7 -> DIR_UPLEFT
        //
        // if mark m_Direction as -1 (invalid)
        // means we don't take consider of turn cost
        int m_Direction;

    private:
        AStarPathFinder *m_Finder;

    private:
        AStarPathFinderNode() = default;
        AStarPathFinderNode(int nX, int nY, int nDirection, AStarPathFinder *pFinder)
            : m_X(nX)
            , m_Y(nY)
            , m_Direction(nDirection)
            , m_Finder(pFinder)
        {
            condcheck(pFinder);
        }

    public:
        ~AStarPathFinderNode() = default;

    public:
        int X() const
        {
            return m_X;
        }

        int Y() const
        {
            return m_Y;
        }

        int Direction() const
        {
            return m_Direction;
        }

    public:
        float GoalDistanceEstimate(AStarPathFinderNode &rstGoalNode)
        {
            // we use Chebyshev's distance instead of Manhattan distance
            // since we allow max step size as 1, 2, 3, and for optimal solution

            // to make A-star algorithm admissible
            // we need h(x) never over-estimate the distance

            auto nMaxStep   = (m_Finder->MaxStep());
            auto nXDistance = (std::labs(rstGoalNode.X() - X()) + (nMaxStep - 1)) / nMaxStep;
            auto nYDistance = (std::labs(rstGoalNode.Y() - Y()) + (nMaxStep - 1)) / nMaxStep;

            return std::max<float>(nXDistance, nYDistance);
        }

        bool IsGoal(AStarPathFinderNode &rstGoalNode)
        {
            return (X() == rstGoalNode.X()) && (Y() == rstGoalNode.Y());
        }

        bool GetSuccessors(AStarSearch<AStarPathFinderNode> *pAStarSearch, AStarPathFinderNode *pParentNode)
        {
            static const int nDX[] = { 0, +1, +1, +1,  0, -1, -1, -1};
            static const int nDY[] = {-1, -1,  0, +1, +1, +1,  0, -1};

            for(int nStepIndex = 0; nStepIndex < ((m_Finder->MaxStep() > 1) ? 2 : 1); ++nStepIndex){
                for(int nDirIndex = 0; nDirIndex < 8; ++nDirIndex){
                    int nNewX = X() + nDX[nDirIndex] * ((nStepIndex == 0) ? m_Finder->MaxStep() : 1);
                    int nNewY = Y() + nDY[nDirIndex] * ((nStepIndex == 0) ? m_Finder->MaxStep() : 1);

                    if(true
                            && pParentNode
                            && pParentNode->X() == nNewX
                            && pParentNode->Y() == nNewY){
                        continue;
                    }

                    // when add a successor we always check it's distance between the ParentNode
                    // means for m_MoveChecker(x0, y0, x1, y1) we guarentee that (x1, y1) inside propor distance to (x0, y0)

                    if(m_Finder->m_MoveChecker(X(), Y(), nNewX, nNewY)){
                        AStarPathFinderNode stFinderNode {nNewX, nNewY, nDirIndex, m_Finder};
                        pAStarSearch->AddSuccessor(stFinderNode);
                    }
                }
            }

            // seems we have to always return true
            // return false means out of memory in the code
            return true;
        }

        float GetCost(AStarPathFinderNode &rstNode)
        {
            int nSrcX = X();
            int nSrcY = Y();
            int nDstX = rstNode.X();
            int nDstY = rstNode.Y();

            if(m_Finder->m_MoveCost){
                auto fCost = m_Finder->m_MoveCost(nSrcX, nSrcY, nDstX, nDstY);
                if(fCost >= 0.00){
                    auto nOldDirIndex = Direction();
                    auto nNewDirIndex = PathFind::GetDirection(nSrcX, nSrcY, nDstX, nDstY) - (DIR_NONE + 1);
                    if(true
                            && (nOldDirIndex >= 0) && (nOldDirIndex < 8)
                            && (nNewDirIndex >= 0) && (nNewDirIndex < 8)){

                        static const float fTurnCost[]
                        {
                            0.00,   // 0
                            1.00,   // 1
                            2.00,   // 2
                            3.00,   // 3
                            4.00,   // 4
                            5.00,   // 5
                            6.00,   // 6
                            7.00,   // 7
                        };

                        auto nDDirIndex = ((nNewDirIndex - nOldDirIndex) + 8) % 8;
                        fCost += fTurnCost[std::min<int>(nDDirIndex, 8 - nDDirIndex)];
                    }
                    return (float)(fCost);
                }
            }
            return 1.00;
        }

        bool IsSameState(AStarPathFinderNode &rstNode)
        {
            return (X() == rstNode.X()) && (Y() == rstNode.Y());
        }
};

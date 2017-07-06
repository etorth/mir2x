/*
 * =====================================================================================
 *
 *       Filename: pathfinder.hpp
 *        Created: 03/28/2017 17:04:54
 *  Last Modified: 07/05/2017 22:34:25
 *
 *    Description: A-Star algorithm for path finding
 *
 *                 if we have a prefperence, we should make it by the cost function,
 *                 rather than hack the source to make a priority
 *
 *                 support jump on the map with step size as (1, 2, 3)
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
            assert(m_MoveChecker);
            assert(m_MoveCost);
            assert(false
                    || (m_MaxStep == 1)
                    || (m_MaxStep == 2)
                    || (m_MaxStep == 3));
        }

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
       int X() const { return m_X; }
       int Y() const { return m_Y; }

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
                for(int nIndex = 0; nIndex < 8; ++nIndex){
                    int nNewX = X() + nDX[nIndex] * ((nStepIndex == 0) ? m_Finder->MaxStep() : 1);
                    int nNewY = Y() + nDY[nIndex] * ((nStepIndex == 0) ? m_Finder->MaxStep() : 1);

                    if(true
                            && pParentNode
                            && pParentNode->X() == nNewX
                            && pParentNode->Y() == nNewY){ continue; }

                    // when add a successor we always check it's distance between the ParentNode
                    // means for m_MoveChecker(x0, y0, x1, y1) we guarentee that (x1, y1) inside propor distance to (x0, y0)

                    if(m_Finder->m_MoveChecker(X(), Y(), nNewX, nNewY)){
                        AStarPathFinderNode stFinderNode {nNewX, nNewY, m_Finder};
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
            if(m_Finder->m_MoveCost){
                auto fCost = m_Finder->m_MoveCost(X(), Y(), rstNode.X(), rstNode.Y());
                return (fCost >= 0.00) ? (float)(fCost) : 1.00;
            }else{
                return 1.00;
            }
        }

        bool IsSameState(AStarPathFinderNode &rstNode)
        {
            return (X() == rstNode.X()) && (Y() == rstNode.Y());
        }
};

namespace PathFind
{
    struct PathNode final
    {
        int X;
        int Y;

        PathNode(int nX, int nY)
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

    int MaxReachNode(const PathFind::PathNode *, size_t, size_t);
}

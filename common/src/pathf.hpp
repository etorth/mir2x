#pragma once
#include <tuple>
#include <cmath>
#include <array>
#include <vector>
#include <optional>
#include <functional>

#include "fsa.h"
#include "stlastar.h"
#include "totype.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"
#include "magicrecord.hpp" // DCCastRange

namespace pathf
{
    struct PathNode final
    {
        int X = -1;
        int Y = -1;

        bool operator == (const PathNode & node)
        {
            return (node.X == X) && (node.Y == Y);
        }

        bool eq(int argX, int argY) const
        {
            return X == argX && Y == argY;
        }
    };
}

namespace pathf
{
    inline bool hopValid(int maxStep, int srcX, int srcY, int dstX, int dstY)
    {
        if(maxStep >= 1 && maxStep <= 3){
            const int distance = mathf::LDistance2(srcX, srcY, dstX, dstY);
            return false
                || distance == 1
                || distance == 2
                || distance == 1 * maxStep * maxStep
                || distance == 2 * maxStep * maxStep;
        }
        return false;
    }

    inline bool dirValid(int direction)
    {
        return direction >= DIR_BEGIN && direction < DIR_END;
    }

    inline const char *dirName(int direction)
    {
        switch(direction){
            case DIR_UP       : return "DIR_UP";
            case DIR_UPRIGHT  : return "DIR_UPRIGHT";
            case DIR_RIGHT    : return "DIR_RIGHT";
            case DIR_DOWNRIGHT: return "DIR_DOWNRIGHT";
            case DIR_DOWN     : return "DIR_DOWN";
            case DIR_DOWNLEFT : return "DIR_DOWNLEFT";
            case DIR_LEFT     : return "DIR_LEFT";
            case DIR_UPLEFT   : return "DIR_UPLEFT";
            default           : return nullptr;
        }
    }

    inline int getBackDir(int direction)
    {
        switch(direction){
            case DIR_UP        : return DIR_DOWN;
            case DIR_DOWN      : return DIR_UP;
            case DIR_LEFT      : return DIR_RIGHT;
            case DIR_RIGHT     : return DIR_LEFT;
            case DIR_UPLEFT    : return DIR_DOWNRIGHT;
            case DIR_UPRIGHT   : return DIR_DOWNLEFT;
            case DIR_DOWNLEFT  : return DIR_UPRIGHT;
            case DIR_DOWNRIGHT : return DIR_UPLEFT;
            default            : return DIR_NONE;
        }
    }

    inline int getNextDir(int dir, int diff = 1)
    {
        fflassert(dirValid(dir), dir);

        constexpr int dirCount = DIR_END - DIR_BEGIN;
        return DIR_BEGIN + (((((dir - DIR_BEGIN) + diff) % dirCount) + dirCount) % dirCount);
    }

    inline int getDirDiff(int srcDir, int dstDir) // always return clock-wise
    {
        fflassert(dirValid(srcDir), srcDir);
        fflassert(dirValid(dstDir), dstDir);

        constexpr int dirCount = DIR_END - DIR_BEGIN;
        return (((dstDir - srcDir) % dirCount) + dirCount) % dirCount;
    }

    inline int getDirAbsDiff(int srcDir, int dstDir) // clock-wise or anti-clock-wise, take minimal
    {
        const auto dirDiff = getDirDiff(srcDir, dstDir);
        return std::min<int>(DIR_END - DIR_BEGIN - dirDiff, dirDiff);
    }

    inline std::tuple<int, int> getFrontGLoc(int x, int y, int dir, int length = 1)
    {
        constexpr static int dx[] = { 0, +1, +1, +1,  0, -1, -1, -1};
        constexpr static int dy[] = {-1, -1,  0, +1, +1, +1,  0, -1};

        fflassert(dirValid(dir), dir);
        return
        {
            x + length * dx[dir - DIR_BEGIN],
            y + length * dy[dir - DIR_BEGIN],
        };
    }

    inline std::tuple<int, int> getBackGLoc(int x, int y, int dir, int length = 1)
    {
        return getFrontGLoc(x, y, dir, -1 * length);
    }

    inline int getOffDir(int argSrcX, int argSrcY, int argDstX, int argDstY)
    {
        constexpr static int offDir[][3]
        {
            {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
            {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
            {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT},
        };

        const int dx = (argDstX > argSrcX) - (argDstX < argSrcX);
        const int dy = (argDstY > argSrcY) - (argDstY < argSrcY);

        return offDir[dy + 1][dx + 1];
    }
}

namespace pathf
{
    int getDir4 (int /* x */, int /* y */);
    int getDir8 (int /* x */, int /* y */);
    int getDir16(int /* x */, int /* y */);

    std::tuple<int, int> getDir4Off (int /* dir */, int /* distance */);
    std::tuple<int, int> getDir8Off (int /* dir */, int /* distance */);
    std::tuple<int, int> getDir16Off(int /* dir */, int /* distance */);

    std::tuple<int, int> getDirOff(int /* x */, int /* y */, int /* distance */);

    bool inDCCastRange(const DCCastRange &, int, int, int, int);
}

namespace pathf
{
    // 1. profiling shows A-star is very expensive, about 1/3 runtime time are spent in A-star search
    // 2. current A-star algorithm has bug to support the turn-cost
    // 3. check this page for jump-point-search algorithm: https://harablog.wordpress.com/2011/09/07/jump-point-search/

    class AStarPathFinder;
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
            // if m_direction is invalid, we don't take consider of turn cost
            int m_direction;

        private:
            AStarPathFinder *m_finder;

        private:
            AStarPathFinderNode() = default;
            AStarPathFinderNode(int argX, int argY, int argDir, AStarPathFinder *argFinder)
                : m_X(argX)
                , m_Y(argY)
                , m_direction(argDir)
                , m_finder(argFinder)
            {
                fflassert(m_finder);
                fflassert(dirValid(m_direction));
            }

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
                return m_direction;
            }

        public:
            float GoalDistanceEstimate(                                    const AStarPathFinderNode &) const;
            bool  IsGoal              (                                    const AStarPathFinderNode &) const;
            bool  GetSuccessors       (AStarSearch<AStarPathFinderNode> *, const AStarPathFinderNode *) const;
            float GetCost             (                                    const AStarPathFinderNode &) const;
            bool  IsSameState         (                                    const AStarPathFinderNode &) const;
    };

    class AStarPathFinder: public AStarSearch<AStarPathFinderNode>
    {
        private:
            friend class AStarPathFinderNode;

        private:
            const std::function<std::optional<double>(int, int, int, int, int)> m_oneStepCost;

        private:
            const int m_maxStep;

        private:
            bool m_hasPath;

        public:
            AStarPathFinder(std::function<std::optional<double>(int, int, int, int, int)> fnOneStepCost, int argMaxStepSize)
                : AStarSearch<AStarPathFinderNode>()
                , m_oneStepCost(std::move(fnOneStepCost))
                , m_maxStep(argMaxStepSize)
                , m_hasPath(false)
            {
                fflassert(m_oneStepCost);
                fflassert(m_maxStep >= 1, m_maxStep);
                fflassert(m_maxStep <= 3, m_maxStep);
            }

        public:
            ~AStarPathFinder()
            {
                if(m_hasPath){
                    FreeSolutionNodes();
                }
                EnsureMemoryFreed();
            }

        protected:
            int maxStep() const
            {
                return m_maxStep;
            }

        public:
            bool hasPath() const
            {
                return m_hasPath;
            }

        public:
            bool search(int, int, int, int, int, size_t searchCount = 0);

        public:
            std::vector<pathf::PathNode> getPathNode()
            {
                if(!hasPath()){
                    return {};
                }

                std::vector<pathf::PathNode> result;
                if(auto node = GetSolutionStart()){
                    result.emplace_back(node->X(), node->Y());
                }
                else{
                    return {};
                }

                while(auto node = GetSolutionNext()){
                    result.emplace_back(node->X(), node->Y());
                }

                return result;
            }

        public:
            template<size_t N> std::tuple<std::array<pathf::PathNode, N>, size_t> getFirstNPathNode()
            {
                static_assert(N >= 2);
                std::array<pathf::PathNode, N> result {};

                if(!hasPath()){
                    return {result, 0};
                }

                if(auto node = GetSolutionStart()){
                    result[0] = {node->X(), node->Y()};
                }
                else{
                    return {result, 0};
                }

                for(size_t i = 1; i < result.size(); ++i){
                    if(auto node = GetSolutionNext()){
                        result[i] = {node->X(), node->Y()};
                    }
                    else{
                        return {result, i};
                    }
                }
                return {result, result.size()};
            }
    };
}

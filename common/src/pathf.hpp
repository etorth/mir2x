#pragma once
#include <tuple>
#include <cmath>
#include <array>
#include <queue>
#include <vector>
#include <optional>
#include <algorithm>
#include <functional>

#include "mathf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"
#include "magicrecord.hpp" // DCCastRange
#include "parallel_hashmap/phmap.h"

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
    // check for D-star algorithm:
    //
    //     https://en.wikipedia.org/wiki/D*
    //
    // check for A-star with JPS algorithm:
    //
    //     https://harablog.wordpress.com/2011/09/07/jump-point-search/
    //
    // implementation based on: https://en.wikipedia.org/wiki/A*_search_algorithm

    class AStarPathFinder
    {
        private:
            struct InnNode final
            {
                int64_t x   : 29;
                int64_t y   : 29;
                int64_t dir :  6;

                bool eq(int argX, int argY) const
                {
                    return this->x == argX && this->y== argY;
                }

                bool eq(int argX, int argY, int argDir) const
                {
                    return eq(argX, argY) && this->dir == argDir;
                }

                bool operator == (const InnNode & param) const noexcept
                {
                    return eq(param.x, param.y, param.dir);
                }
            };

            struct InnNodeHash final
            {
                size_t operator() (const InnNode &node) const noexcept
                {
                    return 0uz
                        + ((size_t)(node.x  ) << (29 + 6))
                        + ((size_t)(node.y  ) << (     6))
                        + ((size_t)(node.dir) << (     0));
                }
            };

            struct InnPQNode final
            {
                InnNode node;
                double  f;

                bool operator < (const InnPQNode &param) const noexcept
                {
                    return this->f > param.f;
                }
            };

            class InnPQ: public std::priority_queue<InnPQNode>
            {
                private:
                    phmap::flat_hash_set<InnNode, InnNodeHash> m_has;

                public:
                    bool has(const InnNode &node) const
                    {
                        return m_has.find(node) != m_has.end();
                    }

                public:
                    void add(const InnPQNode &node)
                    {
                        if(m_has.insert(node.node).second){
                            this->push(node);
                        }
                        else{
                            throw fflvalue(node.node.x, node.node.y, pathf::dirName(node.node.dir), node.f);
                        }
                    }

                public:
                    InnPQNode pick()
                    {
                        fflassert(!this->empty());
                        fflassert(!m_has.empty());

                        const auto t = top();
                        const auto p = m_has.find(t.node);

                        fflassert(p != m_has.end());

                        this->pop();
                        m_has.erase(p);

                        return t;
                    }

                public:
                    void clear()
                    {
                        m_has.clear();
                        this->c.clear();
                    }
            };

        private:
            const int m_maxStep;

        private:
            // give very close weight for StepSize = 1 and StepSize = maxStep to prefer bigger hops
            // but have to make Weight(maxStep) = Weight(1) + dW ( > 0 ), reason:
            //       for maxStep = 3 and path as following:
            //                       A B C D E
            //       if I want to move (A->C), we can do (A->B->C) and (A->D->C)
            //       then if there are of same weight I can't prefer (A->B->C)
            //
            //       but for maxStep = 2 I don't have this issue
            // actually for dW < Weight(1) is good enough

            // cost :   valid  :     1.00
            //        occupied :   100.00
            //         invalid : 10000.00
            //
            // we should have cost(invalid) >> cost(occupied), otherwise
            //          XXAXX
            //          XOXXX
            //          XXBXX
            // path (A->O->B) and (A->X->B) are of equal cost

            // if can't go through we return the infinite
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
            const std::function<std::optional<double>(int, int, int, int, int)> m_oneStepCost;

        private:
            InnNode m_srcNode {};

        private:
            int m_dstX = 0;
            int m_dstY = 0;

        private:
            phmap::flat_hash_map<InnNode, double, InnNodeHash> m_g;
            phmap::flat_hash_map<InnNode, InnNode, InnNodeHash> m_prevSet;

        private:
            InnPQ m_openSet;

        public:
            AStarPathFinder(int argMaxStepSize, std::function<std::optional<double>(int, int, int, int, int)> fnOneStepCost)
                : m_maxStep(argMaxStepSize)
                , m_oneStepCost(std::move(fnOneStepCost))
            {
                fflassert(m_oneStepCost);
                fflassert(m_maxStep >= 1, m_maxStep);
                fflassert(m_maxStep <= 3, m_maxStep);
            }

        public:
            int maxStep() const
            {
                return m_maxStep;
            }

        public:
            bool hasPath() const
            {
                return findLastNode().has_value();
            }

        public:
            std::optional<bool> search(int, int, int, int, int, size_t searchCount = 0);

        public:
            std::vector<pathf::PathNode> getPathNode() const;

        private:
            bool checkGLoc(int, int     ) const;
            bool checkGLoc(int, int, int) const;

        private:
            std::optional<InnNode> findLastNode() const;

        private:
            double h(const InnNode &) const;
    };
}

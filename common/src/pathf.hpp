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

    inline int getRandDir()
    {
        return mathf::rand<int>(DIR_BEGIN, DIR_END - 1);
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

            class InnPQ: public phmap::flat_hash_map<InnNode, double, InnNodeHash>
            {
                public:
                    void update(const InnPQNode &node)
                    {
                        // some A-star tutorial says assignment is much rare than insertion
                        // my measurement is about 75% ~ 90% are insertion, so 10% ~ 25% assignment, it's not rare but much less than insertion
                        this->insert_or_assign(node.node, node.f);
                    }

                public:
                    InnPQNode pick()
                    {
                        // linearly find the minNode by O(n), makes pick() slow while update() is O(1)
                        // based on the fact that 1 pick() needs 8 or 16 update()

                        fflassert(!this->empty());
                        auto p = std::min_element(this->begin(), this->end(), [](const auto &x, const auto &y)
                        {
                            return x.second < y.second;
                        });

                        const InnPQNode minNode
                        {
                            .node = p->first,
                            .f = p->second,
                        };

                        this->erase(p);
                        return minNode;
                    }
            };

        private:
            // why sometime path planning needs to ignore first turn?
            // during planning, path finder respects turn by a small cost which helps to smooth the path
            // but when a character start to move, usually we would like to ignore the first turn, it can be a direct turn-back
            //
            // +---+---+---+
            // |   | A |   |   in game we take  hop cost as: N * 0.10 + 1.0, N is jump size: 1, 2, 3
            // +---+---+---+                   turn ocst as: D * 0.01      , D is diff of direction: 0, 1, 2, 3, 4
            // |   |   |   |
            // +---+---+---+
            // |   |   |   |   we need to plan path from A -> B
            // +---+---+---+   the result depends on the initial direction when character at A
            // | B |   |   |
            // +---+---+---+
            //
            // case 1: when character at A initial direction is DIR_RIGHT:
            //
            //  +---+---+---+
            //  |   | A |   |
            //  +---+---+---+
            //  |   |   | o |  cost = 0.01 + 1.10 + 0.02 + 1.20
            //  +---+---+---+
            //  |   | o |   |
            //  +---+---+---+
            //  | B |   |   |
            //  +---+---+---+
            //
            // for this case, what we would expect is:
            //
            //  +---+---+---+
            //  |   | A |   |
            //  +---+---+---+
            //  |   | o |   |  cost = 0.02 + 1.20 + 0.01 + 1.10
            //  +---+---+---+
            //  |   | o |   |  this shows two path actually has same cost
            //  +---+---+---+  if we would liek the algorithm to prefer this one, we need to ignore the initial turn
            //  | B |   |   |  means don't consider the first turn as a sharp-turn in the path
            //  +---+---+---+
            //
            //
            //
            // case 2: when character at A initial direction is any other than DIR_RIGHT:
            //
            //  +---+---+---+
            //  |   | A |   |
            //  +---+---+---+
            //  | o |   |   |  cost = 0.01 * D + 1.10 + 1.20
            //  +---+---+---+
            //  | o |   |   |  prettry expected
            //  +---+---+---+  the path is prettry smooth
            //  | B |   |   |
            //  +---+---+---+
            //
            // the way to ignore the first turn is same as how we ignore direction at dst point
            // we connect all possible (m_srcNode.x, m_srcNode.y, [DIR_BEGIN, DIR_END)) to an imaginary src node
            const bool m_checkFirstTurn;

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
            // special dst node that can never be reached by stepping
            // it has zero cost to all (m_dstX, m_dstY, [DIR_BEGIN ~ DIR_END)) for multi-targeting
            const InnNode m_dstDrainNode
            {
                .x = 0,
                .y = 0,
                .dir = DIR_NONE, // invalid direction, un-reachable by stepping
            };

        private:
            int m_dstX = 0;
            int m_dstY = 0;

        private:
            phmap::flat_hash_map<InnNode, double, InnNodeHash> m_g;
            phmap::flat_hash_map<InnNode, InnNode, InnNodeHash> m_prevSet;

        private:
            InnPQ m_openSet;

        public:
            AStarPathFinder(bool argCheckFirstTurn, int argMaxStepSize, std::function<std::optional<double>(int, int, int, int, int)> fnOneStepCost)
                : m_checkFirstTurn(argCheckFirstTurn)
                , m_maxStep(argMaxStepSize)
                , m_oneStepCost(std::move(fnOneStepCost))
            {
                fflassert(m_oneStepCost);
                fflassert(m_maxStep >= 1, m_maxStep);
                fflassert(m_maxStep <= 3, m_maxStep);
            }

        public:
            bool checkFirstTurn() const
            {
                return m_checkFirstTurn;
            }

        public:
            int maxStep() const
            {
                return m_maxStep;
            }

        public:
            bool hasPath() const
            {
                return m_prevSet.find(m_dstDrainNode) != m_prevSet.end();
            }

        public:
            std::optional<bool> search(int, int, int, int, int, size_t searchCount = 0);

        public:
            std::vector<pathf::PathNode> getPathNode() const;

        private:
            bool checkGLoc(int, int     ) const;
            bool checkGLoc(int, int, int) const;

        private:
            double h(const InnNode &) const;
            void updatePath(const InnNode &, const InnNode &, double, double);
    };
}

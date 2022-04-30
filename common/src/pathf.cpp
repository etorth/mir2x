#include <cfloat>
#include "pathf.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"

namespace
{
    constexpr int g_dir4Off[][2]
    {
       { 0, -1},
       { 1,  0},
       { 0,  1},
       {-1,  0},
    };

    constexpr double g_dir8Off[][2]
    {
        { 0.000000000000000, -1.000000000000000},
        { 0.707106781186547, -0.707106781186547},
        { 1.000000000000000,  0.000000000000000},
        { 0.707106781186547,  0.707106781186547},
        { 0.000000000000000,  1.000000000000000},
        {-0.707106781186547,  0.707106781186548},
        {-1.000000000000000,  0.000000000000000},
        {-0.707106781186547, -0.707106781186547},
    };

    constexpr double g_dir16Off[][2]
    {
        { 0.000000000000000, -1.000000000000000},
        { 0.382683432365090, -0.923879532511287},
        { 0.707106781186547, -0.707106781186547},
        { 0.923879532511287, -0.382683432365090},
        { 1.000000000000000,  0.000000000000000},
        { 0.923879532511287,  0.382683432365090},
        { 0.707106781186547,  0.707106781186547},
        { 0.382683432365090,  0.923879532511287},
        { 0.000000000000000,  1.000000000000000},
        {-0.382683432365090,  0.923879532511287},
        {-0.707106781186547,  0.707106781186548},
        {-0.923879532511287,  0.382683432365090},
        {-1.000000000000000,  0.000000000000000},
        {-0.923879532511287, -0.382683432365090},
        {-0.707106781186547, -0.707106781186547},
        {-0.382683432365090, -0.923879532511287},
    };

    template<typename T, size_t N> size_t findNearestPoint(const T (&d)[N], int x, int y)
    {
        if(x == 0 && y == 0){
            throw fflerror("invalid direction (x = 0, y = 0)");
        }

        size_t index = -1;
        double distance = DBL_MAX;

        // normalization of (x, y) is optional
        // just for better stablity

        const double len = mathf::LDistance<double>(0, 0, x, y);
        const double xnorm = to_df(x) / len;
        const double ynorm = to_df(y) / len;

        for(size_t i = 0; i < N; ++i){
            if(const double curr = mathf::LDistance2<double>(d[i][0], d[i][1], xnorm, ynorm); distance > curr){
                index = i;
                distance = curr;
            }
        }
        return index;
    }
}

int pathf::getDir4(int x, int y)
{
    if(x == 0 && y == 0){
        return -1;
    }
    return to_d(findNearestPoint(g_dir4Off, x, y));
}

int pathf::getDir8(int x, int y)
{
    if(x == 0 && y == 0){
        return -1;
    }
    return to_d(findNearestPoint(g_dir8Off, x, y));
}

int pathf::getDir16(int x, int y)
{
    if(x == 0 && y == 0){
        return -1;
    }
    return to_d(findNearestPoint(g_dir16Off, x, y));
}

std::tuple<int, int> pathf::getDir4Off(int dirIndex, int d)
{
    if(dirIndex >= 0 && dirIndex < 4){
        return {to_d(std::lround(g_dir4Off[dirIndex][0] * d)), to_d(std::lround(g_dir4Off[dirIndex][1] * d))};
    }
    throw fflerror("direction index is not in [0, 4): %d", dirIndex);
}

std::tuple<int, int> pathf::getDir8Off(int dirIndex, int d)
{
    if(dirIndex >= 0 && dirIndex < 8){
        return {to_d(std::lround(g_dir8Off[dirIndex][0] * d)), to_d(std::lround(g_dir8Off[dirIndex][1] * d))};
    }
    throw fflerror("direction index is not in [0, 8): %d", dirIndex);
}

std::tuple<int, int> pathf::getDir16Off(int dirIndex, int d)
{
    if(dirIndex >= 0 && dirIndex < 16){
        return {to_d(std::lround(g_dir16Off[dirIndex][0] * d)), to_d(std::lround(g_dir16Off[dirIndex][1] * d))};
    }
    throw fflerror("direction index is not in [0, 16): %d", dirIndex);
}

std::tuple<int, int> pathf::getDirOff(int x, int y, int distance)
{
    if(x == 0 && y == 0){
        throw fflerror("invalid direction (x = 0, y = 0)");
    }

    const double r = to_df(distance) / std::sqrt(1.0 * x * x + 1.0 * y * y);
    return
    {
        to_d(std::lround(x * r)),
        to_d(std::lround(y * r)),
    };
}

bool pathf::inDCCastRange(const DCCastRange &r, int x0, int y0, int x1, int y1)
{
    fflassert(r);
    switch(r.type){
        case CRT_DIR:
            {
                const int dx = std::abs(x0 - x1);
                const int dy = std::abs(y0 - y1);

                const int dmax = std::max<int>(dx, dy);
                const int dmin = std::min<int>(dx, dy);

                return false
                    || (dmin == dmax && dmax <= r.distance)
                    || (dmin == 0    && dmax <= r.distance);
            }
        case CRT_LONG:
            {
                return true;
            }
        case CRT_LIMITED:
            {
                return mathf::LDistance2(x0, y0, x1, y1) <= r.distance * r.distance;
            }
        default:
            {
                throw fflvalue(r.type);
            }
    }
}

void pathf::AStarPathFinder::expand_f()
{
    // search direction: ---------------->---------------
    // moving direction: prevNode -> currNode -> nextNode

    const auto currNode = m_cost_PQ_f.pick();
    const auto prevNode = [&currNode, this]() -> std::optional<pathf::AStarPathFinder::InnNode>
    {
        if(currNode.node.eq(m_srcNode.x, m_srcNode.y)){
            return {};
        }

        if(const auto p = m_parentSet_f.find(currNode.node); p != m_parentSet_f.end()){
            return p->second;
        }
        throw fflerror("intermiediate node has no parent: (%d, %d, %s)", currNode.node.x, currNode.node.y, pathf::dirName(currNode.node.dir));
    }();

    for(const auto stepSize: m_stepSizeList){
        for(const auto d: getDirDiffList(m_checkTurn == 0 ? 1 : 8)){
            const auto nextDir = pathf::getNextDir(currNode.node.dir, d);
            const auto [nextX, nextY] = pathf::getFrontGLoc(currNode.node.x, currNode.node.y, nextDir, stepSize);
            fflassert(checkGLoc(nextX, nextY, nextDir), nextX, nextY, nextDir);

            const pathf::AStarPathFinder::InnNode nextNode
            {
                .x   = nextX,
                .y   = nextY,
                .dir = nextDir,
            };

            if(prevNode.has_value() && prevNode.value() == nextNode){
                continue; // don't go back
            }

            const auto hopCost = m_oneStepCost(currNode.node.x, currNode.node.y, m_checkTurn == 0 ? nextNode.dir : currNode.node.dir, nextNode.x, nextNode.y);
            if(!hopCost.has_value()){
                continue; // can not reach
            }

            fflassert(hopCost.value() >= 0.0, hopCost.value());
            const auto reducedCost = hopCost.value() - pf(currNode.node.x, currNode.node.y) + pf(nextNode.x, nextNode.y);

            fflassert(reducedCost >= 0.0, reducedCost);
            if(auto p = m_cost_f.find(nextNode); p == m_cost_f.end() || currNode.cost + reducedCost < p->second){
                if(p == m_cost_f.end()){
                    m_cost_f.try_emplace(nextNode, currNode.cost + reducedCost);
                }
                else{
                    p->second = currNode.cost + reducedCost;
                }

                m_parentSet_f[nextNode] = currNode.node;
                m_cost_PQ_f.update(pathf::AStarPathFinder::InnPQNode
                {
                    .node = nextNode,
                    .cost = currNode.cost + reducedCost,
                });
            }

            if(const auto p = m_cost_r.find(nextNode); p != m_cost_r.end()){
                updateDoneCost(nextNode, currNode.cost + reducedCost + p->second);
            }
        }
    }
}

void pathf::AStarPathFinder::expand_r()
{
    // search direction: ----------------<---------------
    // moving direction: fromNode -> currNode -> prevNode

    const auto currNode = m_cost_PQ_r.pick();
    const auto prevNode = [&currNode, this]() -> std::optional<pathf::AStarPathFinder::InnNode>
    {
        if(currNode.node.eq(m_dstX, m_dstY)){
            return {};
        }

        if(const auto p = m_parentSet_r.find(currNode.node); p != m_parentSet_r.end()){
            return p->second;
        }
        throw fflerror("intermiediate node has no parent: (%d, %d, %s)", currNode.node.x, currNode.node.y, pathf::dirName(currNode.node.dir));
    }();

    for(const auto stepSize: m_stepSizeList){
        for(const auto dFrom: getDirDiffList(m_checkTurn == 0 ? 8 : 1)){
            // need to match from direction
            // if ignore turn completely, all 8 directions can be used
            const auto fromDir = pathf::getNextDir(currNode.node.dir, dFrom);
            const auto [fromX, fromY] = pathf::getBackGLoc(currNode.node.x, currNode.node.y, fromDir, stepSize);

            checkGLoc(fromX, fromY);
            for(const auto d: getDirDiffList(m_checkTurn == 0 ? 1 : 8)){
                const pathf::AStarPathFinder::InnNode fromNode
                {
                    .x   = fromX,
                    .y   = fromY,
                    .dir = ((m_checkTurn == 0) ? DIR_BEGIN : pathf::getNextDir(currNode.node.dir, d)),
                };

                if(prevNode.has_value() && prevNode.value() == fromNode){
                    continue; // don't go back
                }

                const auto hopCost = m_oneStepCost(fromNode.x, fromNode.y, (m_checkTurn == 0) ? fromDir : fromNode.dir, currNode.node.x, currNode.node.y);
                if(!hopCost.has_value()){
                    continue; // can not reach
                }

                fflassert(hopCost.value() >= 0.0, hopCost.value());
                const auto reducedCost = hopCost.value() - pr(currNode.node.x, currNode.node.y) + pr(fromNode.x, fromNode.y);

                fflassert(reducedCost >= 0.0, reducedCost);
                if(auto p = m_cost_r.find(fromNode); p == m_cost_r.end() || currNode.cost + reducedCost < p->second){
                    if(p == m_cost_r.end()){
                        m_cost_r.try_emplace(fromNode, currNode.cost + reducedCost);
                    }
                    else{
                        p->second = currNode.cost + reducedCost;
                    }

                    m_parentSet_r[fromNode] = currNode.node;
                    m_cost_PQ_r.update(pathf::AStarPathFinder::InnPQNode
                    {
                        .node = fromNode,
                        .cost = currNode.cost + reducedCost,
                    });
                }

                if(const auto p = m_cost_f.find(fromNode); p != m_cost_f.end()){
                    updateDoneCost(fromNode, currNode.cost + reducedCost + p->second);
                }
            }
        }
    }
}

pathf::AStarPathFinder::PathFindResult pathf::AStarPathFinder::search(int srcX, int srcY, int srcDir, int dstX, int dstY, size_t searchCount)
{
    fflassert(checkGLoc(srcX, srcY, srcDir), srcX, srcY, srcDir);
    fflassert(checkGLoc(dstX, dstY), dstX, dstY);

    m_srcNode = pathf::AStarPathFinder::InnNode
    {
        .x   = srcX,
        .y   = srcY,
        .dir = ((m_checkTurn == 0) ? DIR_BEGIN : srcDir),
    };

    fflassert(!m_srcNode.eq(dstX, dstY), srcX, srcY, srcDir, dstX, dstY);

    m_dstX = dstX;
    m_dstY = dstY;

    m_pi_f_s = pi(m_srcNode.x, m_srcNode.y, m_dstX     , m_dstY     );
    m_pi_r_t = pi(m_dstX     , m_dstY     , m_srcNode.x, m_srcNode.y);

    m_parentSet_f.clear();
    m_parentSet_r.clear();

    m_cost_PQ_f.clear();
    m_cost_PQ_r.clear();

    m_doneCost.reset();
    m_doneNode.reset();

    for(const auto d: getDirDiffList(m_checkTurn == 1 ? 8 : 1)){
        const pathf::AStarPathFinder::InnNode firstNode
        {
            .x   = m_srcNode.x,
            .y   = m_srcNode.y,
            .dir = pathf::getNextDir(m_srcNode.dir, d),
        };

        m_cost_f[firstNode] = 0.0;
        m_cost_PQ_f.update(pathf::AStarPathFinder::InnPQNode
        {
            .node = firstNode,
            .cost = 0.0,
        });
    }

    for(const auto d: getDirDiffList(m_checkTurn == 0 ? 1 : 8)){
        const pathf::AStarPathFinder::InnNode lastNode
        {
            .x   = m_dstX,
            .y   = m_dstY,
            .dir = pathf::getNextDir(m_srcNode.dir, d),
        };

        m_cost_r[lastNode] = 0.0;
        m_cost_PQ_r.update(pathf::AStarPathFinder::InnPQNode
        {
            .node = lastNode,
            .cost = 0.0,
        });
    }

    const auto fnCheckStop = [this]() -> bool
    {
        return m_cost_PQ_f.top().cost + m_cost_PQ_r.top().cost >= m_doneCost.value_or(DBL_MAX);
    };

    for(size_t c = 0; (searchCount <= 0 || c < searchCount) && !m_cost_PQ_f.empty() && !m_cost_PQ_r.empty(); ++c){
        expand_f();
        if(m_cost_PQ_f.empty()){
            break;
        }

        if(fnCheckStop()){
            break;
        }

        expand_r();
        if(m_cost_PQ_r.empty()){
            break;
        }

        if(fnCheckStop()){
            break;
        }
    }

    return {hasPath(), false};
}

std::vector<pathf::PathNode> pathf::AStarPathFinder::getPathNode() const
{
    fflassert(hasPath());
    const auto fnAppendParentNode = [](const auto &startNode, int stopNodeX, int stopNodeY, const auto &parentSet, auto &result)
    {
        if(startNode.eq(stopNodeX, stopNodeY)){
            return;
        }

        auto currNode = startNode;
        for(auto p = parentSet.find(currNode); p != parentSet.end(); p = parentSet.find(currNode)){
            result.push_back(pathf::PathNode
            {
                .X = to_d(p->second.x),
                .Y = to_d(p->second.y),
            });

            if(p->second.eq(stopNodeX, stopNodeY)){
                return;
            }

            currNode = p->second;
        }

        throw fflerror("intermiediate node has no parent: (%d, %d, %s)", currNode.x, currNode.y, pathf::dirName(currNode.dir));
    };

    std::vector<pathf::PathNode> result;
    fnAppendParentNode(m_doneNode.value(), m_srcNode.x, m_srcNode.y, m_parentSet_f, result);

    std::reverse(result.begin(), result.end());
    result.push_back(pathf::PathNode
    {
        .X = to_d(m_doneNode.value().x),
        .Y = to_d(m_doneNode.value().y),
    });

    fnAppendParentNode(m_doneNode.value(), m_dstX, m_dstY, m_parentSet_r, result);
    return result;
}

bool pathf::AStarPathFinder::checkGLoc(int x, int y) const
{
    return checkGLoc(x, y, DIR_BEGIN);
}

bool pathf::AStarPathFinder::checkGLoc(int x, int y, int dir) const
{
    if(pathf::dirValid(dir)){
        const pathf::AStarPathFinder::InnNode node
        {
            .x   = x,
            .y   = y,
            .dir = dir,
        };

        if(true
                && node.x == x
                && node.y == y
                && node.dir == dir){
            return true;
        }
    }
    return false;
}

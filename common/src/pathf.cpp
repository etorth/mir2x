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

double pathf::AStarPathFinder::h(const InnNode &node) const
{
    // use Chebyshev's distance instead of Manhattan distance
    // since we allow max step size as 1, 2, 3, and for optimal solution

    // to make A-star algorithm admissible
    // we need to make h(x) never over-estimate the distance

    // also h(x) is consistent, although the implementation doesn't require it
    // check: https://en.wikipedia.org/wiki/Consistent_heuristic

    const auto dx = std::labs(node.x - m_dstX);
    const auto dy = std::labs(node.y - m_dstY);

    return std::max<double>(
    {
        1.0 * ((dx / m_maxStep) + (dx % m_maxStep)), // take jump if can, then use move
        1.0 * ((dy / m_maxStep) + (dy % m_maxStep)), //
    });
}

std::optional<bool> pathf::AStarPathFinder::search(int srcX, int srcY, int srcDir, int dstX, int dstY, size_t searchCount)
{
    fflassert(checkGLoc(srcX, srcY, srcDir), srcX, srcY, srcDir);
    fflassert(checkGLoc(dstX, dstY), dstX, dstY);

    m_srcNode = InnNode
    {
        .x = srcX,
        .y = srcY,
        .dir = srcDir,
    };

    fflassert(!m_srcNode.eq(m_dstX, m_dstY), srcX, srcY, srcDir, dstX, dstY);

    m_dstX = dstX;
    m_dstY = dstY;

    m_g.clear();
    m_prevSet.clear();
    m_openSet.clear();

    m_g[m_srcNode] = 0.0;
    m_openSet.add(pathf::AStarPathFinder::InnPQNode
    {
        .node = m_srcNode,
        .f = h(m_srcNode),
    });

    for(size_t c = 0; (searchCount <= 0 || c < searchCount) && !m_openSet.empty(); ++c){
        const auto currNode = m_openSet.pick();
        if(currNode.node.eq(m_dstX, m_dstY)){
            return true;
        }

        const auto distanceJump = {m_maxStep, 1}; // move and jump
        const auto distanceMove = {           1}; // move only
        const auto prevNode = [&currNode, this]() -> std::optional<pathf::AStarPathFinder::InnNode>
        {
            // all other node in m_openSet mush have parent, except src node
            // src node can NOT have parent

            if(currNode.node == m_srcNode){
                return {};
            }

            if(const auto p = m_prevSet.find(currNode.node); p != m_prevSet.end()){
                return p->second;
            }
            throw fflerror("node in open set has no parent: (%d, %d, %s)", currNode.node.x, currNode.node.y, pathf::dirName(currNode.node.dir));
        }();

        for(const auto distance: (m_maxStep > 1) ? distanceJump : distanceMove){
            for(int nextDir = DIR_BEGIN; nextDir < DIR_END; ++nextDir){
                const auto [nextX, nextY] = pathf::getFrontGLoc(currNode.node.x, currNode.node.y, nextDir, distance);
                fflassert(checkGLoc(nextX, nextY, nextDir), nextX, nextY, nextDir);

                const InnNode nextNode
                {
                    .x = nextX,
                    .y = nextY,
                    .dir = nextDir,
                };

                if(prevNode.has_value() && prevNode.value() == nextNode){
                    continue; // don't go back
                }

                const auto hopCost = m_oneStepCost(currNode.node.x, currNode.node.y, currNode.node.dir, nextNode.x, nextNode.y);
                if(!hopCost.has_value()){
                    continue; // can not reach
                }

                fflassert(hopCost.value() >= 0.0, hopCost.value());
                const auto nextNode_g = hopCost.value() + m_g[currNode.node];

                // here can drop the check: pg->second > nextNode_g
                // because h() is consistent, check: https://en.wikipedia.org/wiki/Consistent_heuristic

                if(const auto pg = m_g.find(nextNode); (pg == m_g.end()) || (pg->second > nextNode_g)){
                    m_g[nextNode] = nextNode_g;
                    m_prevSet[nextNode] = currNode.node;

                    if(!m_openSet.has(nextNode)){
                        m_openSet.add(InnPQNode
                        {
                            .node = nextNode,
                            .f = nextNode_g + h(nextNode),
                        });
                    }
                }
            }
        }
    }

    if(m_openSet.empty()){
        return false;
    }
    return {};
}

std::vector<pathf::PathNode> pathf::AStarPathFinder::getPathNode() const
{
    fflassert(hasPath());
    std::vector<pathf::PathNode> result;

    auto currNode = findLastNode().value();
    auto p = m_prevSet.find(currNode);

    while(p != m_prevSet.end()){
        result.push_back(pathf::PathNode
        {
            .X = to_d(p->second.x),
            .Y = to_d(p->second.y),
        });

        if(p->second == m_srcNode){
            std::reverse(result.begin(), result.end());
            return result;
        }

        currNode = p->second;
        p = m_prevSet.find(currNode);
    }
    throw fflerror("intermiediate node has parent: (%d, %d, %s)", currNode.x, currNode.y, pathf::dirName(currNode.dir));
}

bool pathf::AStarPathFinder::checkGLoc(int x, int y) const
{
    return checkGLoc(x, y, DIR_BEGIN);
}

bool pathf::AStarPathFinder::checkGLoc(int x, int y, int dir) const
{
    if(pathf::dirValid(dir)){
        const InnNode node
        {
            .x = x,
            .y = y,
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

std::optional<pathf::AStarPathFinder::InnNode> pathf::AStarPathFinder::findLastNode() const
{
    for(int dir = DIR_BEGIN; dir < DIR_END; ++dir){
        const pathf::AStarPathFinder::InnNode currNode
        {
            .x = m_dstX,
            .y = m_dstY,
            .dir = dir,
        };

        if(m_prevSet.count(currNode)){
            return currNode;
        }
    }
    return {};
}

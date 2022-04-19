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
                throw fflreach();
            }
    }
}

float pathf::AStarPathFinderNode::GoalDistanceEstimate(const AStarPathFinderNode &node) const
{
    // use Chebyshev's distance instead of Manhattan distance
    // since we allow max step size as 1, 2, 3, and for optimal solution

    // to make A-star algorithm admissible
    // we need to make h(x) never over-estimate the distance

    const auto maxStep = m_finder->maxStep();
    const auto dx = std::labs(node.X() - X());
    const auto dy = std::labs(node.Y() - Y());

    return std::max<float>(
    {
        to_f((dx / maxStep) + (dx % maxStep)), // take jump if can, then use move
        to_f((dy / maxStep) + (dy % maxStep)), //
    });
}

bool pathf::AStarPathFinderNode::IsGoal(const AStarPathFinderNode &goalNode) const
{
    return (X() == goalNode.X()) && (Y() == goalNode.Y()); // don't check direction
}

bool pathf::AStarPathFinderNode::GetSuccessors(AStarSearch<AStarPathFinderNode> *searcher, const AStarPathFinderNode *parent) const
{
    const auto distanceJump = {m_finder->maxStep(), 1}; // move and jump
    const auto distanceMove = {                     1}; // move only

    for(const auto distance: (m_finder->maxStep() > 1) ? distanceJump : distanceMove){
        for(int dir = DIR_BEGIN; dir < DIR_END; ++dir){
            const auto [newX, newY] = pathf::getFrontGLoc(X(), Y(), dir, distance);
            if(true
                    && parent
                    && parent->X() == newX
                    && parent->Y() == newY){ // don't go back
                continue;
            }

            // when add a successor we always check it's distance between the ParentNode
            // means for m_moveChecker(x0, y0, x1, y1) we guarentee that (x1, y1) inside propor distance to (x0, y0)

            if(m_finder->m_oneStepCost(X(), Y(), Direction(), newX, newY).has_value()){
                AStarPathFinderNode node
                {
                    newX,
                    newY,
                    dir,
                    m_finder,
                };
                searcher->AddSuccessor(node); // searcher interface requires lvalue-ref
            }
        }
    }

    // need to always return true
    // return false means out of memory in the astar-algorithm template code
    return true;
}

// give very close weight for StepSize = 1 and StepSize = maxStep to prefer bigger hops
// but I have to make Weight(maxStep) = Weight(1) + dW ( > 0 ), reason:
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

float pathf::AStarPathFinderNode::GetCost(const AStarPathFinderNode &node) const
{
    const auto cost = m_finder->m_oneStepCost(X(), Y(), Direction(), node.X(), node.Y());
    fflassert(cost.has_value() && cost.value() >= 0.0f);
    return cost.value();
}

bool pathf::AStarPathFinderNode::IsSameState(const AStarPathFinderNode &node) const
{
    return (X() == node.X()) && (Y() == node.Y()) && (Direction() == node.Direction());
}

bool pathf::AStarPathFinder::search(int srcX, int srcY, int srcDir, int dstX, int dstY, size_t searchCount)
{
    AStarPathFinderNode srcNode {srcX, srcY,    srcDir, this};
    AStarPathFinderNode dstNode {dstX, dstY, DIR_BEGIN, this};

    SetStartAndGoalStates(srcNode, dstNode);

    m_hasPath = false;
    for(size_t c = 0; (searchCount <= 0) || (c < searchCount); ++c){
        if(const auto searchState = SearchStep(); searchState == AStarSearch<AStarPathFinderNode>::SEARCH_STATE_SUCCEEDED){
            m_hasPath = true;
            break;
        }
        else if(searchState == AStarSearch<AStarPathFinderNode>::SEARCH_STATE_SEARCHING){
            continue;
        }
        else{
            break;
        }
    }
    return m_hasPath;
}

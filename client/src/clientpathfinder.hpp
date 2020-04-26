/*
 * =====================================================================================
 *
 *       Filename: clientpathfinder.hpp
 *        Created: 03/28/2017 21:13:11
 *    Description:
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
#include <map>
#include "pathfinder.hpp"

class ClientPathFinder final: public AStarPathFinder
{
    private:
        friend class ProcessRun;

    private:
        const bool m_CheckGround;

    private:
        const int m_CheckCreature;

    private:
        mutable std::map<uint64_t, int> m_Cache;

    public:
        ClientPathFinder(bool, int, int);
       ~ClientPathFinder() = default;

    private:
       int GetGrid(int, int) const;
};

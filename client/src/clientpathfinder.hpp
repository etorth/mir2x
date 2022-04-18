#pragma once
#include <unordered_map>
#include "pathfinder.hpp"
#include "lochashtable.hpp"

class ProcessRun;
class ClientPathFinder final: public AStarPathFinder
{
    private:
        friend class ProcessRun;

    private:
        const ProcessRun * const m_proc;

    private:
        const bool m_checkGround;

    private:
        const int m_checkCreature;

    private:
        mutable LocHashTable<int> m_cache;

    public:
        ClientPathFinder(const ProcessRun *, bool, int, int);

    private:
        int getGrid(int, int) const;
};

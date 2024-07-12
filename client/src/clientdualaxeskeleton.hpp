#pragma once
#include "dbcomid.hpp"
#include "clientmonster.hpp"

class ClientDualAxeSkeleton: public ClientMonster
{
    public:
        ClientDualAxeSkeleton(uint64_t, ProcessRun *, const ActionNode &);

    protected:
        bool onActionAttack(const ActionNode &) override;
};

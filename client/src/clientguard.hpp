#pragma once
#include "clientmonster.hpp"

class ClientGuard: public ClientMonster
{
    public:
        ClientGuard(uint64_t, ProcessRun *, const ActionNode &);

    public:
        bool parseAction(const ActionNode &) override;
};

#pragma once
#include "fflerror.hpp"
#include "clientmonster.hpp"

class ClientAntHealer: public ClientMonster
{
    public:
        ClientAntHealer(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"蚂蚁道士"));
        }
};

#pragma once
#include "fflerror.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerShipwreckLord final: public Monster
{
    public:
        ServerShipwreckLord(ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"霸王教主"), mapPtr, argX, argY, argDir, masterUID)
        {}

    protected:
        corof::long_jmper updateCoroFunc() override;
};

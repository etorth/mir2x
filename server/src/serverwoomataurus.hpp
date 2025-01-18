#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerWoomaTaurus final: public Monster
{
    public:
        ServerWoomaTaurus(ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"沃玛教主"), mapPtr, argX, argY, argDir, masterUID)
        {}

    protected:
        corof::eval_poller<> updateCoroFunc() override;

    protected:
        DamageNode getAttackDamage(int dc, int) const override
        {
            switch(dc){
                case DBCOM_MAGICID(u8"沃玛教主_电光"):
                    {
                        return MagicDamage
                        {
                            .magicID = dc,
                            .damage = 25,
                        };
                    }
                case DBCOM_MAGICID(u8"沃玛教主_雷电术"):
                    {
                        return MagicDamage
                        {
                            .magicID = dc,
                            .damage = 15,
                        };
                    }
                default:
                    {
                        throw fflerror("invalid DC: id = %d, name = %s", dc, to_cstr(DBCOM_MAGICRECORD(dc).name));
                    }
            }
        }

    protected:
        void sendThunderBolt(uint64_t);
};

#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerGuard: public Monster
{
    private:
        const int m_standX;
        const int m_standY;
        const int m_standDirection;

    public:
        ServerGuard(const SDInitGuard &);

    protected:
        corof::awaitable<> runAICoro() override;

    private:
        corof::awaitable<int> checkFriend(uint64_t) override;

    protected:
        void onAMAttack(const ActorMsgPack &) override
        {
            // serverguard won't get any damage
        }

    protected:
        bool canMove(bool)   const override;
        bool canAttack(bool) const override;
};

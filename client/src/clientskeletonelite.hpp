#pragma once
#include "fflerror.hpp"
#include "clientmonster.hpp"

class ClientSkeletonElite: public ClientMonster
{
    public:
        ClientSkeletonElite(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"骷髅精灵"));
        }

    public:
        bool onActionDie(const ActionNode &action) override
        {
            const auto result = ClientMonster::onActionDie(action);

            fflassert(result);
            fflassert(m_forcedMotionQueue.back()->type == MOTION_MON_DIE);

            m_forcedMotionQueue.back()->addUpdate(true, [this](MotionNode *) -> bool
            {
                addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"骷髅精灵_死亡火焰", u8"运行")));
                return true;
            });
            return result;
        }
};

#pragma once
#include "clientmonster.hpp"

class ClientShipwreckGuardian: public ClientMonster
{
    public:
        ClientShipwreckGuardian(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"神舰守卫"));
        }

    public:
        bool onActionDie(const ActionNode &action)
        {
            const auto result = ClientMonster::onActionDie(action);

            fflassert(result);
            fflassert(m_forcedMotionQueue.back()->type == MOTION_MON_DIE);

            m_forcedMotionQueue.back()->addUpdate(true, [this](MotionNode *) -> bool
            {
                addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"神舰守卫_死亡溅射", u8"运行")));
                return true;
            });
            return result;
        }
};

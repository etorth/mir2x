#pragma once
#include "clientmonster.hpp"

class ClientLightArmoredGuard: public ClientMonster
{
    public:
        ClientLightArmoredGuard(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc)
        {
            fflassert(isMonster(u8"轻甲守卫"));
            m_currMotion.reset(new MotionNode
            {
                .type = MOTION_MON_STAND,
                .direction = DIR_UP,
                .x = action.x,
                .y = action.y,
            });
        }

    public:
        bool onActionDie(const ActionNode &action) override
        {
            const auto result = ClientMonster::onActionDie(action);

            fflassert(result);
            fflassert(m_forcedMotionQueue.back()->type == MOTION_MON_DIE);

            m_forcedMotionQueue.back()->addUpdate(true, [this](MotionNode *) -> bool
            {
                addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"轻甲守卫_死亡紫雾", u8"运行")));
                return true;
            });
            return result;
        }
};

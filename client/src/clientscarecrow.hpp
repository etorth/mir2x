/*
 * =====================================================================================
 *
 *       Filename: clientscarecrow.hpp
 *        Created: 08/31/2015 08:26:19
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include "clientmonster.hpp"

class ClientScarecrow: public ClientMonster
{
    public:
        ClientScarecrow(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc)
        {
            fflassert(isMonster(u8"稻草人"));
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
                addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"稻草人_死亡火焰", u8"运行")));
                return true;
            });
            return result;
        }
};

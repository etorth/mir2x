#pragma once
#include "clientmonster.hpp"

class ClientBombSpider: public ClientMonster
{
    public:
        ClientBombSpider(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"爆裂蜘蛛"));
        }

        MonsterFrameGfxSeq getFrameGfxSeq(int motion, int dir) const override
        {
            switch(motion){
                case MOTION_MON_STAND:
                    {
                        // 爆裂蜘蛛 would never stop moving
                        // but server can still post ACTION_STAND for position-sync

                        return
                        {
                            .gfxMotionID = MOTION_MON_WALK,
                            .begin = 0,
                            .count = 1,
                        };
                    }
                case MOTION_MON_WALK:
                case MOTION_MON_DIE:
                    {
                        return ClientMonster::getFrameGfxSeq(motion, dir);
                    }
                default:
                    {
                        return {};
                    }
            }
        }

    public:
        bool onActionDie(const ActionNode &action) override
        {
            const auto result = ClientMonster::onActionDie(action);

            fflassert(result);
            fflassert(m_forcedMotionQueue.back()->type == MOTION_MON_DIE);

            m_forcedMotionQueue.back()->addUpdate(true, [this](MotionNode *) -> bool
            {
                addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"爆裂蜘蛛_死亡雾气", u8"运行")));
                return true;
            });
            return result;
        }
};

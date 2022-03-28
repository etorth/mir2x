#include "clientmonster.hpp"
class ClientTree: public ClientMonster
{
    public:
        ClientTree(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc)
        {
            switch(action.type){
                case ACTION_HITTED:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_HITTED,
                            .direction = DIR_BEGIN,
                            .x = action.x,
                            .y = action.y,
                        });
                        break;
                    }
                default:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_STAND,
                            .direction = DIR_BEGIN,
                            .x = action.x,
                            .y = action.y,
                        });
                        break;
                    }
            }
        }

    public:
        MonsterFrameGfxSeq getFrameGfxSeq(int motion, int) const override
        {
            switch(motion){
                case MOTION_MON_HITTED:
                    {
                        return
                        {
                            .gfxMotionID = MOTION_MON_HITTED,
                            .gfxDirectionID = DIR_BEGIN,
                            .count = 2,
                        };
                    }
                default:
                    {
                        return
                        {
                            .gfxMotionID = MOTION_MON_STAND,
                            .gfxDirectionID = DIR_BEGIN,
                            .count = 4,
                        };
                    }
            }
        }

    public:
        bool visible() const override
        {
            return true;
        }
};

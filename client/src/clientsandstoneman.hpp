#pragma once
#include "fixedlocmagic.hpp"
#include "clientmonster.hpp"

class ClientSandStoneMan: public ClientMonster
{
    public:
        ClientSandStoneMan(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc)
        {
            fflassert(isMonster(u8"沙漠石人"));
            switch(action.type){
                case ACTION_SPAWN:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_SPAWN,
                            .direction = directionValid(action.direction) ? action.direction : to_d(DIR_UP),
                            .x = action.x,
                            .y = action.y,
                        });

                        m_currMotion->addUpdate(true, [proc](MotionNode *motionPtr)
                        {
                            if(motionPtr->frame < 9){
                                return false;
                            }

                            proc->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new StoneManSpawnEffect_RUN
                            {
                                motionPtr->x,
                                motionPtr->y,
                                motionPtr->direction - DIR_BEGIN,
                            }));
                            return true;
                        });
                        break;
                    }
                case ACTION_DIE:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_DIE,
                            .direction = directionValid(action.direction) ? action.direction : to_d(DIR_UP),
                            .x = action.x,
                            .y = action.y,
                        });

                        m_currMotion->addUpdate(true, [proc](MotionNode *motionPtr)
                        {
                            if(motionPtr->frame < 4){
                                return false;
                            }

                            proc->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                            {
                                u8"沙漠石人_死亡",
                                u8"运行",
                                motionPtr->x,
                                motionPtr->y,
                            }));
                            return true;
                        });
                        break;
                    }
                default:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_STAND,
                            .direction = directionValid(action.direction) ? action.direction : to_d(DIR_UP),
                            .x = action.x,
                            .y = action.y,
                        });
                        break;
                    }
            }
        }

    public:
        bool onActionDie(const ActionNode &action) override
        {
            const auto [endX, endY, endDir] = motionEndGLoc().at(1);
            for(auto &node: makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED)){
                fflassert(node);
                fflassert(motionValid(node));
                m_forcedMotionQueue.push_back(std::move(node));
            }

            const auto [dieX, dieY, dieDir] = motionEndGLoc().at(1);
            m_forcedMotionQueue.emplace_back(std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_DIE,
                .direction = dieDir,
                .x = dieX,
                .y = dieY,
            }));

            m_forcedMotionQueue.back()->addUpdate(true, [proc = m_processRun](MotionNode *motionPtr)
            {
                proc->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                {
                    u8"沙漠石人_死亡",
                    u8"运行",
                    motionPtr->x,
                    motionPtr->y,
                }));
                return true;
            });
            return true;
        }
};

#pragma once
#include "totype.hpp"
#include "dbcomid.hpp"
#include "clientmonster.hpp"

class ClientLightBoltZombie: public ClientMonster
{
    public:
        ClientLightBoltZombie(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"雷电僵尸"));
            switch(action.type){
                case ACTION_DIE:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_DIE,
                            .direction = directionValid(action.type) ? to_d(action.type) : DIR_UP,
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
                            .direction = directionValid(action.type) ? to_d(action.type) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });
                        break;
                    }
            }
        }

    public:
        bool onActionAttack(const ActionNode &action) override
        {
            const auto [endX, endY, endDir] = motionEndGLoc().at(1);
            m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
            m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_ATTACK0,
                .direction = m_processRun->getAimDirection(action, endDir),
                .x = action.x,
                .y = action.y,
            }));

            m_motionQueue.back()->addTrigger(false, [this](MotionNode *motionPtr) -> bool
            {
                if(motionPtr->frame < 3){
                    return false;
                }

                m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                {
                    u8"雷电僵尸_雷电",
                    u8"运行",
                    currMotion()->x,
                    currMotion()->y,
                    currMotion()->direction - DIR_BEGIN,
                }));
                return true;
            });
            return true;
        }
};

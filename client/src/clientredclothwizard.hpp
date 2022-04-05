#pragma once
#include "fflerror.hpp"
#include "clientmonster.hpp"

class ClientRedClothWizard: public ClientMonster
{
    public:
        ClientRedClothWizard(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"红衣法师"));
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

            m_motionQueue.back()->addTrigger(false, [targetUID = action.aimUID, proc = m_processRun](MotionNode *motionPtr) -> bool
            {
                if(motionPtr->frame < 3){
                    return false;
                }

                if(auto coPtr = proc->findUID(targetUID)){
                    coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"红衣法师_魔法", u8"运行")));
                }
                return true;
            });
            return true;
        }
};

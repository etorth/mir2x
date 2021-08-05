#pragma once
#include "dbcomid.hpp"
#include "clientmonster.hpp"

class ClientWoomaTaurus: public ClientMonster
{
    public:
        ClientWoomaTaurus(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"沃玛教主"));
        }

    protected:
        bool onActionAttack(const ActionNode &action)
        {
            m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_ATTACK0,
                .direction = m_processRun->getAimDirection(action, currMotion()->direction),
                .x = action.x,
                .y = action.y,
            }));

            switch(action.extParam.attack.damageID){
                case DBCOM_MAGICID(u8"沃玛教主_电光"):
                    {
                        m_motionQueue.back()->addTrigger(false, [this](MotionNode *motionPtr) -> bool
                        {
                            if(motionPtr->frame < 1){
                                return false;
                            }

                            m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                            {
                                u8"沃玛教主_电光",
                                u8"运行",
                                currMotion()->x,
                                currMotion()->y,
                                currMotion()->direction - DIR_BEGIN,
                            }));
                            return true;
                        });
                        return true;
                    }
                case DBCOM_MAGICID(u8"沃玛教主_雷电术"):
                    {
                        m_motionQueue.back()->addTrigger(false, [targetUID = action.aimUID, this](MotionNode *motionPtr) -> bool
                        {
                            if(motionPtr->frame < 3){
                                return false;
                            }

                            if(auto coPtr = m_processRun->findUID(targetUID)){
                                coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"沃玛教主_雷电术", u8"运行")));
                            }
                            return true;
                        });
                        return true;
                    }
                default:
                    {
                        throw fflerror("invalid DC: id = %d, name = %s", to_d(action.extParam.attack.damageID), to_cstr(DBCOM_MAGICRECORD(action.extParam.attack.damageID).name));
                    }
            }
        }
};

#include "pathf.hpp"
#include "processrun.hpp"
#include "clientnumawizard.hpp"

bool ClientNumaWizard::onActionAttack_fireBall(const ActionNode &action)
{
    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .direction = m_processRun->getAimDirection(action, currMotion()->direction),
        .x = action.x,
        .y = action.y,
    }));

    m_motionQueue.back()->addTrigger(false, [targetUID = action.aimUID, this](MotionNode *motionPtr) -> bool
    {
        if(motionPtr->frame < 4){
            return false;
        }

        const auto gfx16DirIndex = [targetUID, motionPtr, this]() -> int
        {
            if(auto coPtr = m_processRun->findUID(targetUID)){
                return pathf::getDir16((coPtr->x() - motionPtr->x) * SYS_MAPGRIDXP, (coPtr->y() - motionPtr->y) * SYS_MAPGRIDYP);
            }
            return (motionPtr->direction - DIR_BEGIN) * 2;
        }();

        m_processRun->addFollowUIDMagic(std::unique_ptr<FollowUIDMagic>(new FollowUIDMagic
        {
            u8"诺玛法老_火球术",
            u8"运行",

            motionPtr->x * SYS_MAPGRIDXP,
            motionPtr->y * SYS_MAPGRIDYP,

            gfx16DirIndex,
            gfx16DirIndex,
            20,

            targetUID,
            m_processRun,
        }))->addOnDone([targetUID, proc = m_processRun](BaseMagic *)
        {
            if(auto coPtr = proc->findUID(targetUID)){
                coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"诺玛法老_火球术", u8"结束")));
            }
        });
        return true;
    });
    return true;
}

bool ClientNumaWizard::onActionAttack_thunderBolt(const ActionNode &action)
{
    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .direction = m_processRun->getAimDirection(action, currMotion()->direction),
        .x = action.x,
        .y = action.y,
    }));

    m_motionQueue.back()->addTrigger(false, [targetUID = action.aimUID, this](MotionNode *motionPtr) -> bool
    {
        if(motionPtr->frame < 5){
            return false;
        }

        if(auto coPtr = m_processRun->findUID(targetUID)){
            coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new Thunderbolt()));
        }
        return true;
    });
    return true;
}

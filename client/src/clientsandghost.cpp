#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientsandghost.hpp"

bool ClientSandGhost::onActionSpawn(const ActionNode &action)
{
    fflassert(m_forcedMotionQueue.empty());
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .seq = rollMotionSeq(),
        .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
        .x = action.x,
        .y = action.y,
    });

    m_standMode = false;
    return true;
}

bool ClientSandGhost::onActionStand(const ActionNode &action)
{
    if(finalStandMode() != (bool)(action.extParam.stand.sandGhost.standMode)){
        addActionTransf();
    }
    return true;
}

bool ClientSandGhost::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.extParam.transf.sandGhost.standModeReq);
    if(finalStandMode() != standReq){
        addActionTransf();
    }
    return true;
}

bool ClientSandGhost::onActionAttack(const ActionNode &action)
{
    if(!finalStandMode()){
        addActionTransf();
    }

    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .seq = rollMotionSeq(),
        .direction = m_processRun->getAimDirection(action, currMotion()->direction),
        .x = action.x,
        .y = action.y,
    }));
    return true;
}

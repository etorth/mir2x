#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientrebornzombie.hpp"

void ClientRebornZombie::addActionTransf()
{
    const auto [endX, endY, endDir] = motionEndGLoc(END_FORCED);
    m_forcedMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_APPEAR,
        .direction = endDir,
        .x = endX,
        .y = endY,
    }));

    m_forcedMotionQueue.back()->addUpdate(true, [this](MotionNode *) -> bool
    {
        m_standMode = !m_standMode;
        return true;
    });
}

bool ClientRebornZombie::onActionSpawn(const ActionNode &action)
{
    fflassert(m_forcedMotionQueue.empty());
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
        .x = action.x,
        .y = action.y,
    });

    m_standMode = false;
    return true;
}

bool ClientRebornZombie::onActionStand(const ActionNode &action)
{
    if(finalStandMode() != (bool)(action.extParam.stand.sandGhost.standMode)){
        addActionTransf();
    }
    return true;
}

bool ClientRebornZombie::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.extParam.transf.sandGhost.standModeReq);
    if(finalStandMode() != standReq){
        addActionTransf();
    }
    return true;
}

bool ClientRebornZombie::onActionAttack(const ActionNode &action)
{
    if(!finalStandMode()){
        addActionTransf();
    }

    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .direction = m_processRun->getAimDirection(action, currMotion()->direction),
        .x = action.x,
        .y = action.y,
    }));
    return true;
}

bool ClientRebornZombie::finalStandMode() const
{
    int countTransf = 0;

    // don't need to count current status
    // if current status is MOTION_MON_APPEAR then the m_standMode has already changed
    //
    // the general rule is: we use end frame status as current status
    // i.e. there is a flower bloom animation, then the m_currMotion->type for this whole animation is "BLOOMED"

    // if(m_currMotion->motion == MOTION_MON_APPEAR){
    //     countTransf++;
    // }

    for(const auto &motionPtr: m_forcedMotionQueue){
        if(motionPtr->type == MOTION_MON_APPEAR){
            countTransf++;
        }
    }

    return (bool)(countTransf % 2) ? !m_standMode : m_standMode;
}

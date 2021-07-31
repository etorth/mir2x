#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientevilcentipede.hpp"

bool ClientEvilCentipede::onActionSpawn(const ActionNode &)
{
    fflassert(m_forcedMotionQueue.empty());
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = DIR_BEGIN,
        .x = x(),
        .y = y(),
    });

    m_standMode = false;
    return true;
}

bool ClientEvilCentipede::onActionStand(const ActionNode &action)
{
    if(finalStandMode() != (bool)(action.extParam.stand.evilCentipede.standMode)){
        addActionTransf();
    }
    return true;
}

bool ClientEvilCentipede::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.extParam.transf.evilCentipede.standModeReq);
    if(finalStandMode() != standReq){
        addActionTransf();
    }
    return true;
}

bool ClientEvilCentipede::onActionAttack(const ActionNode &)
{
    if(!finalStandMode()){
        addActionTransf();
    }

    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .direction = DIR_BEGIN,
        .x = x(),
        .y = y(),
    }));
    return true;
}

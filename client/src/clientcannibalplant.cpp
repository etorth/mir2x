/*
 * =====================================================================================
 *
 *       Filename: clientcannibalplant.cpp
 *        Created: 08/31/2015 08:26:19
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "fflerror.hpp"
#include "processrun.hpp"
#include "clienttaodog.hpp"
#include "clientcannibalplant.hpp"

bool ClientCannibalPlant::onActionSpawn(const ActionNode &action)
{
    fflassert(m_forcedMotionQueue.empty());
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = DIR_BEGIN,
        .x = action.x,
        .y = action.y,
    });

    m_standMode = false;
    return true;
}

bool ClientCannibalPlant::onActionStand(const ActionNode &action)
{
    if(finalStandMode() != action.extParam.stand.cannibalPlant.standMode){
        addActionTransf();
    }
    return true;
}

bool ClientCannibalPlant::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.extParam.transf.cannibalPlant.standModeReq);
    if(finalStandMode() != standReq){
        addActionTransf();
    }
    return true;
}

bool ClientCannibalPlant::onActionAttack(const ActionNode &action)
{
    if(!finalStandMode()){
        addActionTransf();
    }

    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .direction = m_processRun->getAimDirection(action, DIR_BEGIN),
        .x = action.x,
        .y = action.y,
    }));
    return true;
}

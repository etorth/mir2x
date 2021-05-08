/*
 * =====================================================================================
 *
 *       Filename: clientpiranhaplant.cpp
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
#include "clientpiranhaplant.hpp"

void ClientPiranhaPlant::addActionTransf()
{
    const auto [endX, endY, endDir] = motionEndPLoc(END_FORCED);
    m_forceMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_APPEAR,
        .direction = endDir,
        .x = endX,
        .y = endY,
    }));

    m_forceMotionQueue.back()->addUpdate(true, [this](MotionNode *) -> bool
    {
        m_standMode = !m_standMode;
        return true;
    });
}

bool ClientPiranhaPlant::onActionSpawn(const ActionNode &action)
{
    fflassert(m_forceMotionQueue.empty());
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

bool ClientPiranhaPlant::onActionStand(const ActionNode &action)
{
    if(finalStandMode() != action.extParam.stand.piranhaPlant.standMode){
        addActionTransf();
    }
    return true;
}

bool ClientPiranhaPlant::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.extParam.transf.piranhaPlant.standModeReq);
    if(finalStandMode() != standReq){
        addActionTransf();
    }
    return true;
}

bool ClientPiranhaPlant::onActionAttack(const ActionNode &action)
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

bool ClientPiranhaPlant::finalStandMode() const
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

    for(const auto &motionPtr: m_forceMotionQueue){
        if(motionPtr->type == MOTION_MON_APPEAR){
            countTransf++;
        }
    }

    return (bool)(countTransf % 2) ? !m_standMode : m_standMode;
}

int ClientPiranhaPlant::gfxID(int motion, int dir) const
{
    if(m_standMode){
        return ClientMonster::gfxID(motion, dir);
    }
    else{
        switch(motion){
            case MOTION_MON_STAND:
                {
                    // gfx redirect
                    // use the single final frame of MOTION_MON_APPEAR to show it hides in the soil
                    return ClientMonster::gfxID(MOTION_MON_APPEAR, DIR_BEGIN);
                }
            case MOTION_MON_APPEAR:
                {
                    return ClientMonster::gfxID(MOTION_MON_APPEAR, DIR_BEGIN);
                }
            default:
                {
                    return -1;
                }
        }
    }
}

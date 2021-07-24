#include "fflerror.hpp"
#include "processrun.hpp"
#include "clienttaodog.hpp"
#include "clientsandghost.hpp"

void ClientSandGhost::addActionTransf()
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

bool ClientSandGhost::onActionSpawn(const ActionNode &action)
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
        .direction = m_processRun->getAimDirection(action, currMotion()->direction),
        .x = action.x,
        .y = action.y,
    }));
    return true;
}

bool ClientSandGhost::finalStandMode() const
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

std::optional<uint32_t> ClientSandGhost::gfxID(int motion, int dir) const
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
                    return ClientMonster::gfxID(MOTION_MON_APPEAR, dir);
                }
            case MOTION_MON_APPEAR:
                {
                    return ClientMonster::gfxID(MOTION_MON_APPEAR, dir);
                }
            default:
                {
                    return {};
                }
        }
    }
}

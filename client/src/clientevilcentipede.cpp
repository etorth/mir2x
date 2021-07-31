#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientevilcentipede.hpp"

ClientEvilCentipede::ClientEvilCentipede(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : ClientStandMonster(uid, proc)
{
    fflassert(isMonster(u8"触龙神"));
    switch(action.type){
        case ACTION_SPAWN:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = DIR_BEGIN,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = false;
                break;
            }
        case ACTION_STAND:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = DIR_BEGIN,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = (bool)(action.extParam.stand.evilCentipede.standMode);
                break;
            }
        case ACTION_ATTACK:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_ATTACK0,
                    .direction = DIR_BEGIN,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true;
                break;
            }
        case ACTION_TRANSF:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_APPEAR,
                    .direction = DIR_BEGIN,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = (bool)(action.extParam.transf.evilCentipede.standModeReq);
                break;
            }
        case ACTION_HITTED:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_HITTED,
                    .direction = DIR_BEGIN,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true; // can't be hitted if stay in the soil
                break;
            }
        default:
            {
                throw fflerror("invalid action: %s", actionName(action));
            }
    }
}

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

bool ClientEvilCentipede::onActionHitted(const ActionNode &)
{
    if(!finalStandMode()){
        addActionTransf();
    }

    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_HITTED,
        .direction = DIR_BEGIN,
        .x = x(),
        .y = y(),
    }));
    return true;
}

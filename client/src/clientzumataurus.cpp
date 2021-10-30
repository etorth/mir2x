#include "pathf.hpp"
#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientzumataurus.hpp"

ClientZumaTaurus::ClientZumaTaurus(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : ClientStandMonster(uid, proc)
{
    fflassert(isMonster(u8"祖玛教主"));
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
                if(action.extParam.stand.zumaTaurus.standMode){
                    m_currMotion.reset(new MotionNode
                    {
                        .type = MOTION_MON_STAND,
                        .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                        .x = action.x,
                        .y = action.y,
                    });
                    m_standMode = true;
                }
                else{
                    m_currMotion.reset(new MotionNode
                    {
                        .type = MOTION_MON_STAND,
                        .direction = DIR_BEGIN,
                        .x = action.x,
                        .y = action.y,
                    });
                    m_standMode = false;
                }
                break;
            }
        case ACTION_ATTACK:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_ATTACK0,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true;
                break;
            }
        case ACTION_MOVE:
            {
                m_currMotion.reset(new MotionNode
                {
                    // use STAND
                    // otherwise need to figure out proper (endX, endY)
                    .type = MOTION_MON_STAND,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
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
                    .type = MOTION_MON_SPAWN,
                    .direction = DIR_BEGIN,
                    .x = action.x,
                    .y = action.y,
                });

                m_currMotion->addTrigger(false, [this](MotionNode *motionPtr) -> bool
                {
                    if(motionPtr->frame < 9){
                        return false;
                    }

                    m_forcedMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                    {
                        .type = MOTION_MON_STAND,
                        .direction = DIR_DOWNLEFT,
                        .x = motionPtr->x,
                        .y = motionPtr->y,
                    }));

                    m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new ZumaTaurusFragmentEffect_RUN
                    {
                        motionPtr->x,
                        motionPtr->y,
                    }));
                    return true;
                });

                m_standMode = true;
                break;
            }
        case ACTION_HITTED:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_HITTED,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
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

bool ClientZumaTaurus::onActionSpawn(const ActionNode &action)
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

bool ClientZumaTaurus::onActionStand(const ActionNode &action)
{
    if(finalStandMode() != (bool)(action.extParam.stand.zumaTaurus.standMode)){
        addActionTransf();
    }
    return true;
}

bool ClientZumaTaurus::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.extParam.transf.zumaTaurus.standModeReq);
    if(finalStandMode() != standReq){
        addActionTransf();
    }
    return true;
}

bool ClientZumaTaurus::onActionAttack(const ActionNode &action)
{
    if(!finalStandMode()){
        addActionTransf();
    }

    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    if(auto coPtr = m_processRun->findUID(action.aimUID)){
        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
        {
            .type = MOTION_MON_ATTACK0,
            .direction = [&action, endDir, coPtr]() -> int
            {
                const auto nX = coPtr->x();
                const auto nY = coPtr->y();
                if(mathf::LDistance2<int>(nX, nY, action.x, action.y) == 0){
                    return endDir;
                }
                return PathFind::GetDirection(action.x, action.y, nX, nY);
            }(),
            .x = action.x,
            .y = action.y,
        }));

        switch(action.extParam.attack.damageID){
            case DBCOM_MAGICID(u8"祖玛教主_火墙"):
                {
                    m_motionQueue.back()->effect = std::unique_ptr<MotionAlignedEffect>(new MotionAlignedEffect
                    {
                        u8"祖玛教主_火墙",
                        u8"启动",
                        this,
                        m_motionQueue.back().get(),
                    });
                    break;
                }
            case DBCOM_MAGICID(u8"祖玛教主_地狱火"):
                {
                    m_motionQueue.back()->effect = std::unique_ptr<MotionAlignedEffect>(new MotionAlignedEffect
                    {
                        u8"祖玛教主_地狱火",
                        u8"启动",
                        this,
                        m_motionQueue.back().get(),
                    });

                    m_motionQueue.back()->addTrigger(false, [action, this](MotionNode *motionPtr) -> bool
                    {
                        if(motionPtr->frame < 4){
                            return false;
                        }

                        const auto standDir = [motionPtr, &action, this]() -> int
                        {
                            if(action.aimUID){
                                if(auto coPtr = m_processRun->findUID(action.aimUID); coPtr && coPtr->getTargetBox()){
                                    if(const auto dir = m_processRun->getAimDirection(action, DIR_NONE); dir != DIR_NONE){
                                        return dir;
                                    }
                                }
                            }
                            return motionPtr->direction;
                        }();

                        const auto castX = motionPtr->endX;
                        const auto castY = motionPtr->endY;

                        for(const auto distance: {1, 2, 3, 4, 5, 6, 7, 8}){
                            m_processRun->addDelay(distance * 100, [standDir, castX, castY, distance, castMapID = m_processRun->mapID(), proc = m_processRun]()
                            {
                                if(proc->mapID() != castMapID){
                                    return;
                                }

                                const auto [aimX, aimY] = pathf::getFrontGLoc(castX, castY, standDir, distance);
                                if(!proc->groundValid(aimX, aimY)){
                                    return;
                                }

                                proc->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new HellFire_RUN
                                {
                                    aimX,
                                    aimY,
                                    standDir,
                                }))->addTrigger([aimX, aimY, proc](MagicBase *magicPtr)
                                {
                                    if(magicPtr->frame() < 10){
                                        return false;
                                    }

                                    proc->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FireAshEffect_RUN
                                    {
                                        aimX,
                                        aimY,
                                        1000,
                                    }));
                                    return true;
                                });
                            });
                        }
                        return true;
                    });
                    break;
                }
            default:
                {
                    throw bad_reach();
                }
        }
    }
    return true;
}

void ClientZumaTaurus::addActionTransf()
{
    ClientStandMonster::addActionTransf();

    fflassert(!m_forcedMotionQueue.empty());
    fflassert(m_forcedMotionQueue.back()->type == MOTION_MON_SPAWN);

    m_forcedMotionQueue.back()->addTrigger(false, [this](MotionNode *motionPtr) -> bool
    {
        if(motionPtr->frame < 9){
            return false;
        }

        m_forcedMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
        {
            .type = MOTION_MON_STAND,
            .direction = DIR_DOWNLEFT,
            .x = motionPtr->x,
            .y = motionPtr->y,
        }));

        m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new ZumaTaurusFragmentEffect_RUN
        {
            motionPtr->x,
            motionPtr->y,
        }));
        return true;
    });
}

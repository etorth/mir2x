#include "pathf.hpp"
#include "dbcomid.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "motioneffect.hpp"
#include "clientminotaurguardian.hpp"

bool ClientMinotaurGuardian::onActionAttack(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);

    switch(const auto magicID = action.extParam.attack.magicID){
        case DBCOM_MAGICID(u8"潘夜右护卫_电魔杖"):
        case DBCOM_MAGICID(u8"潘夜左护卫_火魔杖"):
            {
                m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = MOTION_MON_ATTACK0,
                    .direction = m_processRun->getAimDirection(action, currMotion()->direction),
                    .x = action.x,
                    .y = action.y,
                }));

                m_motionQueue.back()->effect.reset(new MotionSyncEffect(DBCOM_MAGICRECORD(magicID).name, u8"运行", this, m_motionQueue.back().get(), 3));
                m_motionQueue.back()->addTrigger(false, [targetUID = action.aimUID, magicID, this](MotionNode *motionPtr) -> bool
                {
                    if(motionPtr->frame < 4){
                        return false;
                    }

                    if(auto coPtr = m_processRun->findUID(targetUID)){
                        coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(DBCOM_MAGICRECORD(magicID).name, u8"结束")));
                    }
                    return true;
                });
                return true;
            }
        case DBCOM_MAGICID(u8"潘夜右护卫_雷电术"):
            {
                m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = MOTION_MON_ATTACK1,
                    .direction = m_processRun->getAimDirection(action, currMotion()->direction),
                    .x = action.x,
                    .y = action.y,
                }));

                m_motionQueue.back()->addTrigger(false, [targetUID = action.aimUID, this](MotionNode *motionPtr) -> bool
                {
                    if(motionPtr->frame < 4){
                        return false;
                    }

                    if(auto coPtr = m_processRun->findUID(targetUID)){
                        coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new Thunderbolt()));
                    }
                    return true;
                });
                return true;
            }
        case DBCOM_MAGICID(u8"潘夜左护卫_火球术"):
            {
                m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = MOTION_MON_ATTACK1,
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
                        u8"潘夜左护卫_火球术",
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
                            coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"潘夜左护卫_火球术", u8"结束")));
                        }
                    });
                    return true;
                });
                return true;
            }
        default:
            {
                throw fflerror("invalid DC: id = %d, name = %s", to_d(magicID), to_cstr(DBCOM_MAGICRECORD(magicID).name));
            }
    }
}

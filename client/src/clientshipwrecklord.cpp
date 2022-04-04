#include "dbcomid.hpp"
#include "processrun.hpp"
#include "motioneffect.hpp"
#include "clientshipwrecklord.hpp"

bool ClientShipwreckLord::onActionAttack(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);

    switch(const auto magicID = action.extParam.attack.magicID){
        case DBCOM_MAGICID(u8"物理攻击"):
            {
                m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = MOTION_MON_ATTACK0,
                    .seq = rollMotionSeq(),
                    .direction = m_processRun->getAimDirection(action, currMotion()->direction),
                    .x = action.x,
                    .y = action.y,
                }));

                m_motionQueue.back()->effect.reset(new MotionSyncEffect(u8"霸王教主_火刃", u8"运行", this, m_motionQueue.back().get()));
                return true;
            }
        case DBCOM_MAGICID(u8"霸王教主_野蛮冲撞"):
            {
                m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = MOTION_MON_SPELL0,
                    .seq = rollMotionSeq(),
                    .direction = m_processRun->getAimDirection(action, currMotion()->direction),
                    .x = action.x,
                    .y = action.y,
                }));
                return true;
            }
        default:
            {
                throw fflerror("invalid DC: id = %d, name = %s", to_d(magicID), to_cstr(DBCOM_MAGICRECORD(magicID).name));
            }
    }
}

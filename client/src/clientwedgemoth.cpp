#include "pathf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientwedgemoth.hpp"

ClientWedgeMoth::ClientWedgeMoth(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : ClientMonster(uid, proc, action)
{
    fflassert(isMonster(u8"楔蛾"));
    switch(action.type){
        case ACTION_SPAWN:
        case ACTION_STAND:
        case ACTION_HITTED:
        case ACTION_DIE:
        case ACTION_ATTACK:
        case ACTION_MOVE:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = pathf::dirValid(action.type) ? to_d(action.type) : DIR_BEGIN,
                    .x = action.x,
                    .y = action.y,
                });
                break;
            }
        default:
            {
                throw fflerror("invalid initial action: %s", actionName(action.type));
            }
    }
}

bool ClientWedgeMoth::onActionAttack(const ActionNode &action)
{
    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .direction = m_processRun->getAimDirection(action, currMotion()->direction),
        .x = action.x,
        .y = action.y,
    }));

    m_motionQueue.back()->addTrigger(false, [this](MotionNode *motionPtr) -> bool
    {
        if(motionPtr->frame < 5){
            return false;
        }

        m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
        {
            u8"楔蛾_喷毒",
            u8"运行",
            currMotion()->x,
            currMotion()->y,
            currMotion()->direction - DIR_BEGIN,
        }));
        return true;
    });
    return true;
}

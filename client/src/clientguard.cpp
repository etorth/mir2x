/*
 * =====================================================================================
 *
 *       Filename: clientguard.cpp
 *        Created: 08/31/2015 08:26:57
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
#include "clientguard.hpp"

bool ClientGuard::parseAction(const ActionNode &action)
{
    m_lastActive = SDL_GetTicks();
    m_motionQueue.clear();

    switch(action.type){
        case ACTION_JUMP:
        case ACTION_STAND:
        case ACTION_SPAWN:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = action.direction,
                    .x = action.x,
                    .y = action.y,
                });
                return true;
            }
        case ACTION_ATTACK:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_ATTACK0,
                    .direction = m_processRun->getAimDirection(action, m_currMotion->direction),
                    .x = action.x,
                    .y = action.y,
                });
                return true;
            }
        default:
            {
                throw bad_reach();
            }
    }
}

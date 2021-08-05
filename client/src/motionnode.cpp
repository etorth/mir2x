/*
 * =====================================================================================
 *
 *       Filename: motionnode.cpp
 *        Created: 04/05/2017 12:40:09
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
#include <cinttypes>
#include "log.hpp"
#include "mathf.hpp"
#include "colorf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "sdldevice.hpp"
#include "motionnode.hpp"
#include "pngtexoffdb.hpp"

extern Log *g_log;
extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_magicDB;

void MotionNode::runTrigger()
{
    for(auto p = m_triggerList.begin(); p != m_triggerList.end();){
        if((*p)(this)){
            p = m_triggerList.erase(p);
        }
        else{
            p++;
        }
    }
}

void MotionNode::addTrigger(bool addBefore, std::function<bool(MotionNode *)> op)
{
    fflassert(op);
    if(addBefore){
        m_triggerList.push_front(std::move(op));
    }
    else{
        m_triggerList.push_back(std::move(op));
    }
}

void MotionNode::print() const
{
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::motion            = %s", to_cvptr(this), motionName(type)          );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::direction         = %d", to_cvptr(this), direction                 );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::speed             = %d", to_cvptr(this), speed                     );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::x                 = %d", to_cvptr(this), x                         );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::y                 = %d", to_cvptr(this), y                         );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::endX              = %d", to_cvptr(this), endX                      );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::endY              = %d", to_cvptr(this), endY                      );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::frame             = %d", to_cvptr(this), frame                     );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::triggerList::size = %d", to_cvptr(this), to_d(m_triggerList.size()));
}

double MotionNode::frameDelay() const
{
    const auto baseDelay = 1000.0 / to_df(SYS_DEFFPS);
    const auto fspeed = to_df(mathf::bound<int>(speed, SYS_MINSPEED, SYS_MAXSPEED)) / 100.0;
    return baseDelay / fspeed;
}

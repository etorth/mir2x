/*
 * =====================================================================================
 *
 *       Filename: motionnode.hpp
 *        Created: 04/05/2017 12:38:46
 *    Description: for field MotionNode::speed
 *
 *                      means % speed of default speed
 *
 *                 i.e. if default speed is 100 FPS:
 *
 *                      MotionNode::speed :  20 : FPS =  20 : min
 *                                           50 : FPS =  50 : slow
 *                                          100 : FPS = 100 : default
 *                                          200 : FPS = 200 : fast
 *                                          500 : FPS = 500 : max
 *
 *                  currently support speed : 20 ~ 500 => speed x5 or d5
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

#pragma once
#include <list>
#include <memory>
#include <cinttypes>
#include <functional>
#include "mathf.hpp"
#include "motion.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "strf.hpp"
#include "fflerror.hpp"
#include "dbcomid.hpp"
#include "protocoldef.hpp"
#include "motioneffect.hpp"

struct MotionNode final
{
    struct MotionExtParam
    {
        struct MotionSpell
        {
            const uint32_t magicID = 0;
        }
        spell{};

        struct MotionAttack
        {
            const int motion = 0;
        }
        attack{};

        struct MotionDie
        {
            int fadeOut = 0;
        }
        die{};

        struct MotionSwing
        {
            const uint32_t magicID = 0;
        }
        swing{};
    };

    ///////////////////////////////////////////////////////////////////////////
    ////                                                                   ////
    ////          AGGREGATE INITIALIZATION MEMEBER LIST                    ////
    ////                                                                   ////
    ///////////////////////////////////////////////////////////////////////////
    /**/                                                                   /**/
    /**/    const int type = MOTION_NONE;                                  /**/
    /**/    const int seq  = 0;                                            /**/
    /**/                                                                   /**/
    /**/    const int direction = DIR_NONE;                                /**/
    /**/    /***/ int speed     = SYS_DEFSPEED;                            /**/
    /**/                                                                   /**/
    /**/    const int x = -1;                                              /**/
    /**/    const int y = -1;                                              /**/
    /**/                                                                   /**/
    /**/    const int endX = x;                                            /**/
    /**/    const int endY = y;                                            /**/
    /**/                                                                   /**/
    /**/    int frame = 0;                                                 /**/
    /**/    MotionExtParam extParam {};                                    /**/
    /**/    std::unique_ptr<MotionEffect> effect {};                       /**/
    /**/                                                                   /**/
    ///////////////////////////////////////////////////////////////////////////

    // private members
    // make it public to support init by initializer_list
    std::list<std::function<bool(MotionNode *)>> m_triggerList {};

    operator bool () const
    {
        return false
            || ((type >= MOTION_BEGIN)     && (type < MOTION_END))
            || ((type >= MOTION_MON_BEGIN) && (type < MOTION_MON_END))
            || ((type >= MOTION_NPC_BEGIN) && (type < MOTION_NPC_END));
    }

    void runTrigger()
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

    void addTrigger(bool addBefore, std::function<bool(MotionNode *)> op)
    {
        if(op){
            if(addBefore){
                m_triggerList.push_front(std::move(op));
            }
            else{
                m_triggerList.push_back(std::move(op));
            }
        }
    }

    void print(const std::function<void(const std::string &)> &logFunc) const
    {
        if(logFunc){
            logFunc(str_printf("[0x%p]::motion            = %s", to_cvptr(this), motionName(this->type)    ));
            logFunc(str_printf("[0x%p]::seq               = %d", to_cvptr(this), this->seq                 ));
            logFunc(str_printf("[0x%p]::direction         = %d", to_cvptr(this), this->direction           ));
            logFunc(str_printf("[0x%p]::speed             = %d", to_cvptr(this), this->speed               ));
            logFunc(str_printf("[0x%p]::x                 = %d", to_cvptr(this), this->x                   ));
            logFunc(str_printf("[0x%p]::y                 = %d", to_cvptr(this), this->y                   ));
            logFunc(str_printf("[0x%p]::endX              = %d", to_cvptr(this), this->endX                ));
            logFunc(str_printf("[0x%p]::endY              = %d", to_cvptr(this), this->endY                ));
            logFunc(str_printf("[0x%p]::frame             = %d", to_cvptr(this), this->frame               ));
            logFunc(str_printf("[0x%p]::triggerList::size = %d", to_cvptr(this), to_d(m_triggerList.size())));
        }
    }

    double frameDelay() const
    {
        return (1000.0 / to_df(SYS_DEFFPS)) / (to_df(mathf::bound<int>(speed, SYS_MINSPEED, SYS_MAXSPEED)) / 100.0);
    }

    uint64_t getSeqID() const
    {
        return (to_u64(this->type) << 48) | (to_u64(this->seq) << 16);
    }

    uint64_t getSeqFrameID() const
    {
        return (to_u64(this->type) << 48) | (to_u64(this->seq) << 16) | to_u64(this->frame);
    }
};

// for field MotionNode::speed
//
//      means % speed of default speed
//
// i.e. if default speed is 100 FPS:
//
//      MotionNode::speed :  20 : FPS =  20 : min
//                           50 : FPS =  50 : slow
//                          100 : FPS = 100 : default
//                          200 : FPS = 200 : fast
//                          500 : FPS = 500 : max
//
//  currently support speed : 20 ~ 500 => speed x5 or d5

#pragma once
#include <list>
#include <memory>
#include <cstdint>
#include <climits>
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

        struct MotionHitted
        {
            const uint64_t fromUID = 0;
        }
        hitted{};

        struct MotionDie
        {
            int fadeOut = 0; //mutable during update
        }
        die{};

        struct MotionSwing
        {
            const uint32_t magicID = 0;
        }
        swing{};

        struct MotionCombined
        {
            std::vector<std::pair<int, int>> frames;
        }
        combined{};
    };

    ///////////////////////////////////////////////////////////////////////////
    ////                                                                   ////
    ////          AGGREGATE INITIALIZATION MEMEBER LIST                    ////
    ////                                                                   ////
    ///////////////////////////////////////////////////////////////////////////
    /**/                                                                   /**/
    /**/    const int type      =  MOTION_NONE;                            /**/
    /**/    const int direction =     DIR_NONE;                            /**/
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
    // make public to support init by initializer_list

    const uint32_t m_seq = []()
    {
        // if use ClientCreature::m_motionSeqRoller, needs make it mutable
        // because ClientCreature::makeIdleMotion() changes it but which needs const-qualified *this*

        if(static uint32_t rollMotionSeq = 1; rollMotionSeq == UINT32_MAX){
            return rollMotionSeq = 1;
        }
        else{
            return ++rollMotionSeq;
        }
    }();

    std::list<std::function<bool(MotionNode *)>> m_triggerList {};

    // public member functions
    // private member function get implemeted as anonymous lambdas

    operator bool () const
    {
        return false
            || ((type >= MOTION_BEGIN)     && (type < MOTION_END))
            || ((type >= MOTION_MON_BEGIN) && (type < MOTION_MON_END))
            || ((type >= MOTION_NPC_BEGIN) && (type < MOTION_NPC_END))
            || ((type >= MOTION_EXT_BEGIN) && (type < MOTION_EXT_END));
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
            logFunc(str_printf("[%llu]::motion            = %s", to_llu(m_seq), motionName(this->type)    ));
            logFunc(str_printf("[%llu]::direction         = %d", to_llu(m_seq), this->direction           ));
            logFunc(str_printf("[%llu]::speed             = %d", to_llu(m_seq), this->speed               ));
            logFunc(str_printf("[%llu]::x                 = %d", to_llu(m_seq), this->x                   ));
            logFunc(str_printf("[%llu]::y                 = %d", to_llu(m_seq), this->y                   ));
            logFunc(str_printf("[%llu]::endX              = %d", to_llu(m_seq), this->endX                ));
            logFunc(str_printf("[%llu]::endY              = %d", to_llu(m_seq), this->endY                ));
            logFunc(str_printf("[%llu]::frame             = %d", to_llu(m_seq), this->frame               ));
            logFunc(str_printf("[%llu]::triggerList::size = %d", to_llu(m_seq), to_d(m_triggerList.size())));
        }
    }

    double frameDelay() const
    {
        return (1000.0 / to_df(SYS_DEFFPS)) / (to_df(mathf::bound<int>(speed, SYS_MINSPEED, SYS_MAXSPEED)) / 100.0);
    }

    uint64_t getSeqID() const
    {
        return (to_u64(this->type) << 48) | (to_u64(m_seq) << 16);
    }

    uint64_t getSeqFrameID() const
    {
        return (to_u64(this->type) << 48) | (to_u64(m_seq) << 16) | to_u64(this->frame);
    }

    int gfxType(std::optional<int> frameOpt = {}) const
    {
        if(type == MOTION_EXT_COMBINED){
            return extParam.combined.frames.at(frameOpt.value_or(this->frame)).first;
        }
        else{
            return type;
        }
    }

    int gfxFrame(std::optional<int> frameOpt = {}) const
    {
        if(type == MOTION_EXT_COMBINED){
            return extParam.combined.frames.at(frameOpt.value_or(this->frame)).second;
        }
        else{
            return frameOpt.value_or(frame);
        }
    }
};

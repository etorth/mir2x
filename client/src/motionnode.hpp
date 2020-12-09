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
#include <functional>
#include "motion.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
#include "protocoldef.hpp"

struct MotionNode
{
    class MotionEffect
    {
        public:
            virtual void drawShift(int, int, bool) = 0;

        public:
            virtual int frame     () const = 0;
            virtual int frameCount() const = 0;
    };

    // class SwordSwingEffect: public MotionEffect;

    class MagicSpellEffect: public MotionEffect
    {
        protected:
            int m_frame = 0;
            const MagicGfxEntry * const m_gfxEntry;

        public:
            MagicSpellEffect(const MotionNode *);

        public:
            int frame() const override;
            int frameCount() const override;
            void drawShift(int, int, bool) override;
    };

    struct MotionExtParam
    {
        struct MotionSpell
        {
            int magicID = 0;
        }spell;

        struct MotionAttack
        {
            int motion = 0;
        }attack;
    };
    static_assert(std::is_trivially_copyable_v<MotionExtParam>);

    //////////////////////////////////////////////////////////////////
    ////                                                          ////
    ////          AGGREGATE INITIALIZATION MEMEBER LIST           ////
    ////                                                          ////
    //////////////////////////////////////////////////////////////////
    /**/                                                          /**/
    /**/    int type      = MOTION_NONE;                          /**/
    /**/    int direction = DIR_NONE;                             /**/
    /**/    int speed     = SYS_DEFSPEED;                         /**/
    /**/                                                          /**/
    /**/    int x = -1;                                           /**/
    /**/    int y = -1;                                           /**/
    /**/                                                          /**/
    /**/    int endX = x;                                         /**/
    /**/    int endY = y;                                         /**/
    /**/                                                          /**/
    /**/    int frame   = 0;                                      /**/
    /**/    int fadeOut = 0;                                      /**/
    /**/                                                          /**/
    /**/    MotionExtParam param{};                               /**/
    /**/    std::unique_ptr<MotionEffect> effect{};               /**/
    /**/    std::function<void(MotionNode *, bool)> onUpdate{};   /**/
    /**/                                                          /**/
    //////////////////////////////////////////////////////////////////

    operator bool () const
    {
        return false
            || ((type >= MOTION_BEGIN)     && (type < MOTION_END))
            || ((type >= MOTION_MON_BEGIN) && (type < MOTION_MON_END))
            || ((type >= MOTION_NPC_BEGIN) && (type < MOTION_NPC_END));
    }

    void print() const;
    static const char *name(int);
};

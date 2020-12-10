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

class MotionNode;
class MotionEffectBase
{
    protected:
        const MotionNode * const m_motion;

    public:
        MotionEffectBase(const MotionNode *motionPtr)
            : m_motion(motionPtr)
        {
            if(!m_motion){
                throw fflerror("invalid motion bind: nullptr");
            }
        }

    public:
        virtual ~MotionEffectBase() = default;

    public:
        virtual int frame     () const = 0;
        virtual int frameCount() const = 0;

    protected:
        virtual uint32_t frameTexID() const
        {
            return SYS_TEXNIL;
        }

    public:
        virtual void nextFrame() = 0;
        virtual void drawShift(int, int, bool) = 0;
};

class MagicSpellEffect: public MotionEffectBase
{
    protected:
        int m_frame = 0;
        const MagicGfxEntry * const m_gfxEntry;

    public:
        MagicSpellEffect(const MotionNode *);

    public:
        int frame     () const override;
        int frameCount() const override;

    protected:
        uint32_t frameTexID() const override;

    public:
        void nextFrame() override;
        void drawShift(int, int, bool) override;
};

class TaoSumDogEffect: public MagicSpellEffect
{
    // drop some frames
    // summon dog has 19 frames
    // which holds hero stay to wait too long

    public:
        using MagicSpellEffect::MagicSpellEffect;

    private:
        constexpr static uint32_t m_texID[]
        {
            0, 1, 2, 4, 6, 8, 10, 12, 14, 16, 17, 18,
        };

    public:
        int frameCount() const override
        {
            return (int)(std::extent_v<decltype(m_texID)>);
        }

        uint32_t frameTexID() const override
        {
            return m_gfxEntry->gfxID + m_texID[m_frame];
        }
};

class TaoFireFigureEffect: public MagicSpellEffect
{
    public:
        using MagicSpellEffect::MagicSpellEffect;

    public:
        int frameCount() const override
        {
            return MagicSpellEffect::frameCount() + 2;
        }

        uint32_t frameTexID() const override
        {
            if(frame() < MagicSpellEffect::frameCount()){
                return MagicSpellEffect::frameTexID();
            }
            return SYS_TEXNIL;
        }
};

struct MotionNode
{
    struct MotionExtParam
    {
        struct MotionSpell
        {
            int magicID = 0;
            std::unique_ptr<MotionEffectBase> effect;
        }spell;

        struct MotionAttack
        {
            int motion = 0;
        }attack;

        struct MotionDie
        {
            int fadeOut = 0;
        }die;
        
        struct MotionSwing
        {
            int magicID = 0;
            std::unique_ptr<MotionEffectBase> effect;
        }swing;
    };

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
    /**/    int frame = 0;                                        /**/
    /**/    MotionExtParam extParam{};                            /**/
    /**/    std::function<void(MotionNode *)> onUpdate{};         /**/
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

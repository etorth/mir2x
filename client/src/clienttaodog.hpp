/*
 * =====================================================================================
 *
 *       Filename: clienttaodog.hpp
 *        Created: 08/31/2015 08:26:19
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

#pragma once
#include <unordered_map>
#include "dbcomid.hpp"
#include "clientmonster.hpp"

class ClientTaoDog: public ClientMonster
{
    private:
        class DogFire: public AttachMagic
        {
            private:
                const std::unordered_map<int, std::array<int, 2>> *m_dirOffCPtr;

            public:
                DogFire(int gfxDirIndex)
                    : AttachMagic(u8"神兽-喷火", u8"运行", gfxDirIndex)
                    , m_dirOffCPtr([]()
                      {
                          const static std::unordered_map<int, std::array<int, 2>> s_dirOff
                          {
                              {DIR_UP,        { 0,  2}},
                              {DIR_UPRIGHT,   {-3, -3}},
                              {DIR_RIGHT,     {-2, -5}},
                              {DIR_DOWNRIGHT, { 0,  0}},
                              {DIR_DOWN,      { 0, -5}},
                              {DIR_DOWNLEFT,  { 0,  0}},
                              {DIR_LEFT,      { 2, -5}},
                              {DIR_UPLEFT,    { 3, -3}},
                          };
                          return &s_dirOff;
                      }())
                {}

            public:
                void drawShift(int drawOffX, int drawOffY, bool alpha) override
                {
                    const int dir = gfxDirIndex() + DIR_BEGIN;
                    AttachMagic::drawShift(drawOffX + m_dirOffCPtr->at(dir).at(0), drawOffY + m_dirOffCPtr->at(dir).at(1), alpha);
                }
        };

    private:
        bool m_stand;

    public:
        ClientTaoDog(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
            , m_stand(action.ActionParam)
        {
            checkMonsterNameEx(u8"神兽");
        }

    public:
        int motionFrameCount(int motion, int) const override
        {
            if(m_stand){
                switch(motion){
                    case MOTION_MON_STAND   : return  4;
                    case MOTION_MON_WALK    : return  6;
                    case MOTION_MON_ATTACK0 : return  6;
                    case MOTION_MON_HITTED  : return  2;
                    case MOTION_MON_DIE     : return 10;
                    case MOTION_MON_APPEAR  : return 10; // from crawling to stand
                    default                 : return -1;
                }
            }
            else{
                switch(motion){
                    case MOTION_MON_STAND   : return  4;
                    case MOTION_MON_WALK    : return  6;
                    case MOTION_MON_HITTED  : return  2;
                    case MOTION_MON_DIE     : return 10;
                    case MOTION_MON_APPEAR  : return 10; // from non to crawling
                    default                 : return -1;
                }
            }
        }

    public:
        int lookID() const override
        {
            return m_stand ? 0X5A : 0X59;
        }

    protected:
        void onActionSpawn(const ActionNode &action) override
        {
            checkActionSpawnEx(action);
            m_currMotion = MotionNode
            {
                MOTION_MON_APPEAR,
                0,

                [&action]() -> int
                {
                    if(action.Direction >= DIR_BEGIN && action.Direction < DIR_END){
                        return action.Direction;
                    }
                    return DIR_UP;
                }(),

                action.X,
                action.Y,
            };
        }

        void onActionTransf(const ActionNode &action) override
        {
            checkActionTransfEx(action);
            const auto standReq = (bool)(action.ActionParam);

            if(m_stand == standReq){
                return;
            }

            const auto [x, y, dir] = [this]() -> std::array<int, 3>
            {
                if(!m_forceMotionQueue.empty()){
                    return {m_forceMotionQueue.back().endX, m_forceMotionQueue.back().endY, m_forceMotionQueue.back().direction};
                }
                return {m_currMotion.endX, m_currMotion.endY, m_currMotion.direction};
            }();

            m_stand = standReq;
            m_forceMotionQueue.emplace_back(MotionNode
            {
                .type = MOTION_MON_APPEAR,
                .direction = dir,
                .x = x,
                .y = y,
            });
        }

    public:
        bool visible() const override
        {
            return ClientCreature::active();
        }

    public:
        bool updateMotion() override
        {
            if(!ClientCreature::updateMotion()){
                return false;
            }

            if(m_currMotion.type == MOTION_MON_ATTACK0 && m_currMotion.frame == 5 && m_stand){
                addAttachMagic(std::unique_ptr<AttachMagic>(new DogFire
                {
                    m_currMotion.direction - DIR_BEGIN,
                }));
            }
            return true;
        }
};

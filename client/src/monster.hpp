/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
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
#include "uidf.hpp"
#include "client.hpp"
#include "clientmsg.hpp"
#include "dbcomrecord.hpp"
#include "protocoldef.hpp"
#include "creaturemovable.hpp"

class Monster: public CreatureMovable
{
    public:
        static Monster *createMonster(uint64_t, ProcessRun *, const ActionNode &);

    protected:
        Monster(uint64_t, ProcessRun *);

    public:
        bool update(double) override;
        bool draw(int, int, int) override;

    public:
        uint32_t monsterID() const
        {
            return uidf::getMonsterID(UID());
        }

    protected:
        std::tuple<int, int> location() const override;

    public:
        bool parseAction(const ActionNode &);

    public:
        bool motionValid(const MotionNode &) const;

    public:
        bool canFocus(int, int) const override;

    protected:
        int gfxMotionID(int motion) const override
        {
            if((motion > MOTION_MON_NONE) && (motion < MOTION_MON_MAX)){
                return (motion - (MOTION_MON_NONE + 1));
            }
            return -1;
        }

    protected:
        int gfxID(int, int) const;

    public:
        int motionFrameCount(int, int) const override;

    protected:
        MotionNode makeMotionWalk(int, int, int, int, int) const;

    public:
        int maxStep() const override
        {
            return 1;
        }
        int currStep() const override
        {
            return 1;
        }

    public:
        int lookID() const
        {
            if(const auto &mr = DBCOM_MONSTERRECORD(monsterID())){
                return mr.LookID;
            }
            return -1;
        }
};

/*
 * =====================================================================================
 *
 *       Filename: clientmonster.hpp
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

class ClientMonster: public CreatureMovable
{
    public:
        static std::unique_ptr<MotionNode> makeInitMotion(uint32_t, const ActionNode &);

    public:
        ClientMonster(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc)
        {
            if(auto initMotion = ClientMonster::makeInitMotion(uidf::getMonsterID(uid), action)){
                m_currMotion = std::move(initMotion);
                return;
            }
            throw fflerror("failed to assign initial motion");
        }

    protected:
        ClientMonster(uint64_t, ProcessRun *);

    public:
        bool update(double) override;
        void draw(int, int, int) override;

    public:
        uint32_t monsterID() const
        {
            return uidf::getMonsterID(UID());
        }

    protected:
        std::tuple<int, int> location() const override;

    public:
        bool parseAction(const ActionNode &) override;

    public:
        bool motionValid(const std::unique_ptr<MotionNode> &) const;

    protected:
        int gfxMotionID(int motion) const override
        {
            if((motion >= MOTION_MON_BEGIN) && (motion < MOTION_MON_END)){
                return (motion - MOTION_MON_BEGIN);
            }
            return -1;
        }

    protected:
        int gfxID(int, int) const;

    public:
        int motionFrameCount(int, int) const override;

    protected:
        std::unique_ptr<MotionNode> makeWalkMotion(int, int, int, int, int) const;

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
        virtual int lookID() const
        {
            if(const auto &mr = DBCOM_MONSTERRECORD(monsterID())){
                return mr.lookID;
            }
            return -1;
        }

    protected:
        virtual bool onActionDie       (const ActionNode &);
        virtual bool onActionStand     (const ActionNode &);
        virtual bool onActionHitted    (const ActionNode &);
        virtual bool onActionMove      (const ActionNode &);
        virtual bool onActionAttack    (const ActionNode &);
        virtual bool onActionSpawn     (const ActionNode &);
        virtual bool onActionTransf    (const ActionNode &);
        virtual bool onActionSpaceMove2(const ActionNode &);

    protected:
        template<size_t N> void checkMonsterNameEx(const char8_t (&name)[N]) const
        {
            if(monsterID() != DBCOM_MONSTERID(name)){
                throw fflerror("invalid monster name: %s", to_cstr(DBCOM_MONSTERRECORD(monsterID()).name));
            }
        }

    public:
        ClientCreature::TargetBox getTargetBox() const override;
};

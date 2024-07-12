#pragma once
#include <optional>
#include "uidf.hpp"
#include "pathf.hpp"
#include "client.hpp"
#include "clientmsg.hpp"
#include "protocoldef.hpp"
#include "creaturemovable.hpp"

class ClientMonster;
struct MonsterFrameGfxSeq final
{
    const std::optional<int>      gfxLookID {};
    const std::optional<int>    gfxMotionID {};
    const std::optional<int> gfxDirectionID {};

    const int begin = 0;
    const int count = 0;
    const bool reverse = false;

    operator bool() const
    {
        return begin >= 0 && count > 0;
    }

    std::optional<uint32_t> gfxID(const ClientMonster *, std::optional<int> = {}) const;
};

class ClientMonster: public CreatureMovable
{
    public:
        ClientMonster(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc)
        {
            switch(action.type){
                case ACTION_DIE:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_DIE,
                            .direction = pathf::dirValid(action.direction) ? to_d(action.direction) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });

                        if(const auto deathEffectName = str_printf(u8"%s_死亡特效", to_cstr(monsterName())); DBCOM_MAGICID(to_u8cstr(deathEffectName))){
                            m_currMotion->addTrigger(true, [deathEffectName, this](MotionNode *) -> bool
                            {
                                addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(to_u8cstr(deathEffectName), u8"运行")));
                                return true;
                            });
                        }
                        break;
                    }
                default:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_STAND,
                            .direction = pathf::dirValid(action.direction) ? to_d(action.direction) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });
                        break;
                    }
            }
        }

    protected:
        ClientMonster(uint64_t, ProcessRun *);

    public:
        bool update(double) override;

    public:
        void drawFrame(int, int, int, int, bool) override;

    public:
        uint32_t monsterID() const
        {
            return uidf::getMonsterID(UID());
        }

        std::u8string_view monsterName() const
        {
            return DBCOM_MONSTERRECORD(monsterID()).name;
        }

        const auto &getMR() const
        {
            return DBCOM_MONSTERRECORD(monsterID());
        }

        int lookID() const
        {
            return getMR().lookID;
        }

    public:
        uint32_t getSeffID(int) const;

    public:
        int getFrameCount(const MotionNode *motionPtr) const override
        {
            return getFrameGfxSeq(motionPtr->type, motionPtr->direction).count;
        }

    public:
        bool parseAction(const ActionNode &) override;

    public:
        bool motionValid(const std::unique_ptr<MotionNode> &) const override;

    public:
        virtual MonsterFrameGfxSeq getFrameGfxSeq(int, int) const;

    protected:
        std::unique_ptr<MotionNode> makeWalkMotion(int, int, int, int, int) const;

    public:
        int  maxStep() const override;
        int currStep() const override;

    protected:
        virtual bool onActionDie      (const ActionNode &);
        virtual bool onActionStand    (const ActionNode &);
        virtual bool onActionHitted   (const ActionNode &);
        virtual bool onActionJump     (const ActionNode &);
        virtual bool onActionMove     (const ActionNode &);
        virtual bool onActionAttack   (const ActionNode &);
        virtual bool onActionSpawn    (const ActionNode &);
        virtual bool onActionTransf   (const ActionNode &);
        virtual bool onActionSpaceMove(const ActionNode &);

    public:
        ClientCreature::TargetBox getTargetBox() const override;

    protected:
        std::unique_ptr<MotionNode> makeIdleMotion() const override
        {
            return std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_STAND,
                .direction = m_currMotion->direction,
                .x = m_currMotion->endX,
                .y = m_currMotion->endY,
            });
        }

    public:
        bool deadFadeOut() override;

    public:
        static ClientMonster *create(uint64_t, ProcessRun *, const ActionNode &);
};

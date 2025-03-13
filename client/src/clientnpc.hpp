#pragma once
#include <cstdint>
#include <optional>
#include "totype.hpp"
#include "processrun.hpp"
#include "actionnode.hpp"
#include "clientcreature.hpp"

class ClientNPC;
struct NPCFrameGfxSeq final
{
    const int count = 0;

    operator bool() const
    {
        return count > 0;
    }

    std::optional<uint32_t> gfxID(const ClientNPC *, std::optional<int> = {}) const;
};

class ClientNPC: public ClientCreature
{
    public:
        ClientNPC(uint64_t, ProcessRun *, const ActionNode &);

    public:
        uint16_t lookID() const
        {
            return uidf::getNPCID(UID());
        }

    public:
        void drawFrame(int, int, int, int, bool) override;

    public:
        NPCFrameGfxSeq getFrameGfxSeq(int, int) const;

    public:
        int getFrameCount(const MotionNode *motionPtr) const override
        {
            return getFrameGfxSeq(motionPtr->type, motionPtr->direction).count;
        }

    public:
        std::tuple<int, int> location() const override
        {
            return {m_currMotion->endX, m_currMotion->endY};
        }

    public:
        bool parseAction(const ActionNode &) override;

    public:
        bool update(double) override;

    public:
        bool motionValid(const std::unique_ptr<MotionNode> &) const override;

    public:
        bool deadFadeOut() override
        {
            return true; // do nothing
        }

    public:
        bool moveNextMotion() override
        {
            m_currMotion = makeIdleMotion();
            return true;
        }

    public:
        TargetBox getTargetBox() const override;

    protected:
        std::unique_ptr<MotionNode> makeIdleMotion() const override
        {
            return std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_NPC_ACT,
                .direction = m_currMotion->direction,
                .x = m_currMotion->endX,
                .y = m_currMotion->endY,
            });
        }
};

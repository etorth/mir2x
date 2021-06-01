/*
 * =====================================================================================
 *
 *       Filename: clientnpc.hpp
 *        Created: 04/12/2020 12:49:26
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
#include <cstdint>
#include "totype.hpp"
#include "processrun.hpp"
#include "actionnode.hpp"
#include "clientcreature.hpp"

class ClientNPC: public ClientCreature
{
    public:
        ClientNPC(uint64_t, ProcessRun *, const ActionNode &);

    public:
        uint16_t lookID() const
        {
            return uidf::getLookID(UID());
        }

    protected:
        std::optional<uint32_t> gfxID() const;

    public:
        void drawFrame(int, int, int, int, bool) override;

    public:
        FrameSeq motionFrameSeq(int, int) const override;

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

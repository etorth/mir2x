#pragma once
#include <unordered_map>
#include "dbcomid.hpp"
#include "clientmonster.hpp"

// base class for monsters has stand/lying state
// use MOTION_MON_APPEAR to show the transfer gfx, may need gfx redirection

class ClientStandMonster: public ClientMonster
{
    protected:
        bool m_standMode = false;

    public:
        ClientStandMonster(uint64_t uid, ProcessRun *proc)
            : ClientMonster(uid, proc)
        {}

    protected:
        void addActionTransf()
        {
            const auto [endX, endY, endDir] = motionEndGLoc(END_FORCED);
            m_forcedMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_APPEAR,
                .direction = endDir,
                .x = endX,
                .y = endY,
            }));

            m_forcedMotionQueue.back()->addUpdate(true, [this](MotionNode *) -> bool
            {
                m_standMode = !m_standMode;
                return true;
            });
        }

    protected:
        bool finalStandMode() const
        {
            // don't need to count current status
            // if current status is MOTION_MON_APPEAR then the m_standMode has already changed
            //
            // the general rule is: we use end frame status as current status
            // i.e. there is a flower bloom animation, then the m_currMotion->type for this whole animation is "BLOOMED"

            // if(m_currMotion->motion == MOTION_MON_APPEAR){
            //     countTransf++;
            // }

            int countTransf = 0;
            for(const auto &motionPtr: m_forcedMotionQueue){
                if(motionPtr->type == MOTION_MON_APPEAR){
                    countTransf++;
                }
            }
            return (bool)(countTransf % 2) ? !m_standMode : m_standMode;
        }
};

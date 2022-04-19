/*
 * =====================================================================================
 *
 *       Filename: creaturemovable.hpp
 *        Created: 04/25/2020 22:25:20
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
#include <array>
#include <deque>
#include <vector>
#include <cstdint>
#include "motionnode.hpp"
#include "clientcreature.hpp"
#include "clientpathfinder.hpp"

class ProcessRun;
class CreatureMovable: public ClientCreature
{
    protected:
        std::deque<std::unique_ptr<MotionNode>> m_motionQueue;
        std::deque<std::unique_ptr<MotionNode>> m_forcedMotionQueue;

    protected:
        CreatureMovable(uint64_t uid, ProcessRun *pRun)
            : ClientCreature(uid, pRun)
        {}

    protected:
        virtual int  maxStep() const = 0;
        virtual int currStep() const = 0;

    public:
        std::tuple<int, int> location() const override
        {
            return getGLoc()[1];
        }

    protected:
        std::array<std::tuple<int, int>, 3> getGLoc() const;

    protected:
        virtual bool stayIdle() const
        {
            return m_forcedMotionQueue.empty() && m_motionQueue.empty();
        }

    protected:
        // parse a motion path for src -> dst for current creature
        // parameters:
        //      src            : (nX0, nY0)
        //      dst            : (nX1, nY1)
        //      bCheckGround   :
        //      bCheckCreature :
        //
        //  1. src point should be valid grid on map
        //  2. this function allows dst to be invalid location if bCheckGround is not set
        //  3. this function automatically use the minmal time path by calling ClientCreature::MaxStep()
        //  4. check ClientPathFinder for bCheckGround and bCheckCreature
        //
        // return vector size:
        //      0 : error happens
        //      1 : can't find a path and keep standing on (nX0, nY0)
        //     >1 : path found
        virtual std::vector<pathf::PathNode> parseMovePath(
                int, int,       // src
                int, int,       // dst
                bool,           // bCheckGround
                int);           // nCheckCreature

    protected:
        virtual std::unique_ptr<MotionNode> makeWalkMotion(int, int, int, int, int) const = 0;
        std::deque<std::unique_ptr<MotionNode>> makeWalkMotionQueue(int, int, int, int, int);

    protected:
        bool motionQueueValid() const;

    public:
        bool moveNextMotion() override;

    public:
        std::tuple<int, int> getShift(int) const;

    public:
        std::array<std::tuple<int, int, int>, 3> motionEndGLoc() const;

    public:
        virtual void flushForcedMotion();
};

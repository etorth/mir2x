/*
 * =====================================================================================
 *
 *       Filename: standnpc.hpp
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
#include "creature.hpp"
#include "processrun.hpp"
#include "actionnode.hpp"

class StandNPC: public Creature
{
    public:
        StandNPC(uint64_t, ProcessRun *, const ActionNode &);

    public:
        uint16_t lookID() const
        {
            return 0;
        }

    protected:
        int32_t gfxID() const;

    public:
        bool draw(int, int, int) override;

    public:
        int motionFrameCount(int, int) const override;

    private:
        int32_t gfxShadowID(int32_t) const;
        bool canFocus(int , int) const override;
};

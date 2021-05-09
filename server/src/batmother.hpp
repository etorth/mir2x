/*
 * =====================================================================================
 *
 *       Filename: batmother.hpp
 *        Created: 04/07/2016 03:48:41 AM
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
#include "dbcomid.hpp"
#include "monster.hpp"

class BatMother final: public Monster
{
    public:
        BatMother(ServerMap *mapPtr, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"角蝇"), mapPtr, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        void addBat();

    protected:
        corof::long_jmper updateCoroFunc() override;

    protected:
        ActionNode makeActionStand() const override
        {
            return ActionStand
            {
                .x = X(),
                .y = Y(),
                .direction = DIR_BEGIN,
            };
        }
};

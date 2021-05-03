/*
 * =====================================================================================
 *
 *       Filename: piranhaplant.hpp
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

class PiranhaPlant final: public Monster
{
    public:
        PiranhaPlant(ServerMap *mapPtr, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"食人花"), mapPtr, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        corof::long_jmper updateCoroFunc() override;
};

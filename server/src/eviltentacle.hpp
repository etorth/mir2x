/*
 * =====================================================================================
 *
 *       Filename: eviltentacle.hpp
 *        Created: 04/26/2021 02:32:45
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

class EvilTentacle: public Monster
{
    public:
        EvilTentacle(ServerMap *mapPtr, int argX, int argY, int argDir)
            : Monster(DBCOM_MONSTERID(u8"触角神魔"), mapPtr, argX, argY, argDir, 0)
        {}

    protected:
        corof::long_jmper updateCoroFunc() override;
};

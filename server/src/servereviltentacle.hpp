/*
 * =====================================================================================
 *
 *       Filename: servereviltentacle.hpp
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

class ServerEvilTentacle: public Monster
{
    public:
        ServerEvilTentacle(uint32_t monID, ServerMap *mapPtr, int argX, int argY, int argDir)
            : Monster(monID, mapPtr, argX, argY, argDir, 0)
        {
            fflassert(isMonster(u8"触角神魔") || isMonster(u8"爆毒神魔"));
        }

    protected:
        corof::long_jmper updateCoroFunc() override;
};

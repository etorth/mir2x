/*
 * =====================================================================================
 *
 *       Filename: servertaoskeleton.hpp
 *        Created: 04/10/2016 02:32:45
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

class ServerTaoSkeleton final: public Monster
{
    public:
        ServerTaoSkeleton(ServerMap *mapPtr, int argX, int argY, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"变异骷髅"), mapPtr, argX, argY, DIR_DOWNLEFT, masterUID)
        {}
};

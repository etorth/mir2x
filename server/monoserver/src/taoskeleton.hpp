/*
 * =====================================================================================
 *
 *       Filename: taoskeleton.hpp
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

class TaoSkeleton final: public Monster
{
    public:
        TaoSkeleton(ServiceCore *corePtr, ServerMap *mapPtr, int argX, int argY, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"变异骷髅"), corePtr, mapPtr, argX, argY, DIR_DOWNLEFT, masterUID)
        {}
};

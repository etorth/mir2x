/*
 * =====================================================================================
 *
 *       Filename: monsterrecord.hpp
 *        Created: 05/08/2017 16:21:14
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
#include <span>
#include <cstdint>
#include "protocoldef.hpp"

struct MonsterRecord
{
    const char8_t *name = u8"";

    const int level   = 0;
    const int undead  = 0;
    const int tamable = 0;
    const int coolEye = 0;

    const int lookID      = 0;
    const int deadFadeOut = 0;

    const int HP  = 0;
    const int MP  = 0;
    const int hit = 0;
    const int exp = 0;

    const int DC     = 0;
    const int DCMax  = 0;
    const int MDC    = 0;
    const int MDCMax = 0;
    const int AC     = 0;
    const int ACMax  = 0;
    const int MAC    = 0;
    const int MACMax = 0;

    const int ACFire    = 0;
    const int ACIce     = 0;
    const int ACLight   = 0;
    const int ACWind    = 0;
    const int ACHoly    = 0;
    const int ACDark    = 0;
    const int ACPhantom = 0;

    const int walkWait  = 0;
    const int walkSpeed = 0;

    const int attackMode   = 0;
    const int attackWait   = 0;
    const int attackEffect = 0;

    const char8_t *dcNameList[8] = {};

    operator bool() const
    {
        return name && name[0] != '\0';
    }

    auto dcList() const
    {
        for(const auto &p: dcNameList){
            if(!p){
                return std::span<const char8_t * const>(dcNameList, &p);
            }
        }
        return std::span<const char8_t * const>(dcNameList, dcNameList + std::extent_v<decltype(dcNameList)>);
    }
};

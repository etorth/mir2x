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
#include <cstdint>
#include <initializer_list>
#include "protocoldef.hpp"

// behave mode
// monster used this in server side to setup behavior AI
constexpr int BM_DEFAULT = 0;
constexpr int BM_NEUTRAL = 1;
constexpr int BM_GUARD   = 2;

struct MonsterRecord
{
    const char8_t *name = nullptr;

    const int lookID      = 0;
    const int undead      = 0;
    const int tameable    = 0;
    const int view        = 0;  // range of view, zero means blind
    const int coolEye     = 0;
    const int deadFadeOut = 0;

    const int hp  = 0;
    const int mp  = 0;
    const int exp = 0;

    const int  dc[2] = {0, 0};
    const int  mc[2] = {0, 0};
    const int  ac[2] = {0, 0};
    const int mac[2] = {0, 0};

    const int dcHit = 0;
    const int mcHit = 0;

    const int dcDodge = 0;
    const int mcDodge = 0;

    const int hpRecover = 0;
    const int mpRecover = 0;

    struct AddElem
    {
        const int fire    = 0;
        const int ice     = 0;
        const int light   = 0;
        const int wind    = 0;
        const int holy    = 0;
        const int dark    = 0;
        const int phantom = 0;
    };

    const AddElem dcElem {};
    const AddElem acElem {};

    const int behaveMode = 0; // 0 means BM_DEFAULT

    const int walkWait = 0;
    const int walkStep = 0;

    const int attackWait   = 0;
    const int attackEffect = 0;

    const char8_t *dcName = nullptr;
    const char8_t *description = nullptr;

    operator bool() const
    {
        return name && name[0] != '\0';
    }
};

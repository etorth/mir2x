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
#include <tuple>
#include <array>
#include <cstdint>
#include <optional>
#include <initializer_list>
#include "sysconst.hpp"
#include "protocoldef.hpp"

constexpr int MONSEFF_SPAWN  = 0;
constexpr int MONSEFF_STAND  = 1;
constexpr int MONSEFF_ATTACK = 2;
constexpr int MONSEFF_HITTED = 4;
constexpr int MONSEFF_DIE    = 5;

constexpr std::array<std::optional<std::tuple<std::u8string_view, uint32_t>>, SYS_SEFFSIZE> seffMapWrapper(std::initializer_list<std::tuple<uint32_t, std::u8string_view, uint32_t>> seffList)
{
    std::array<std::optional<std::tuple<std::u8string_view, uint32_t>>, SYS_SEFFSIZE> result {};
    for(const auto &[index, name, offset]: seffList){
        if(index < result.size()){
            if(name.empty() || offset < SYS_SEFFSIZE){
                result[index] = std::make_optional<std::tuple<std::u8string_view, uint32_t>>(name, offset);
            }
            else{
                // request to direct to another monster but offset is illegal, ignore
            }
        }
        else{
            // bad index, ignore
        }
    }
    return result;
}

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

    const int shadow      = 1;
    const int deadFadeOut = 0;

    const struct SoundEffectList
    {
        // priority level:
        // 1. ref: redirect to another monster
        // 2. list: seff.list[i] redirect to another monster per entry if has value

        const std::u8string_view ref {};
        const std::array<std::optional<std::tuple<std::u8string_view, uint32_t>>, SYS_SEFFSIZE> list {};
    }
    seff {};

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

    const int ai = 0;           // 0 -> 100 means stupid -> smart
    const int behaveMode = 0;   // 0 means BM_DEFAULT, which is against to player, override by its master's behavior

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

/*
 * =====================================================================================
 *
 *       Filename: itemrecord.hpp
 *        Created: 07/28/2017 17:12:29
 *    Description: for each item I have two GfxID:
 *                      PkgGfxID : when on ground and in inventory
 *                      UseGfxID : when drawing with player body
 *
 *                 why should I introduce all two different IDs?
 *
 *                 Best practice should be one unique GfxID for all items and retrieve
 *                 different set of animations from different archieves using the unique
 *                 GfxID.
 *
 *                 But in ghe graphical libraries, one item may share one GfxID for one
 *                 kind of usage but own its own resource for another usage. i.e. for the
 *                 诅咒屠龙 and 幸运屠龙, when drawing it with human body they are sharing
 *                 one set of animation, but when staying in inventory they have their own
 *                 different animations.
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
#include <string>
#include <cstdint>
#include <string_view>

struct ItemRecord
{
    const char8_t * const name;
    const char8_t * const type;
    const char8_t * const rarity;

    int weight;
    int pkgGfxID;
    int useGfxID;

    const char8_t * const needJob;

    int needLevel;
    int needDC;
    int needSPC;
    int needMDC;
    int needAC;
    int needMAC;

    const char8_t * const description;

    operator bool() const
    {
        return std::u8string_view(name) != u8"";
    }

    constexpr bool hasDBID() const
    {
        using namespace std::literals;
        return false
            || type == u8"鞋"sv
            || type == u8"衣服"sv
            || type == u8"武器"sv
            || type == u8"头盔"sv
            || type == u8"项链"sv
            || type == u8"戒指"sv
            || type == u8"手镯"sv;
    }
};

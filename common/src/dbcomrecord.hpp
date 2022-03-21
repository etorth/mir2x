/*
 * =====================================================================================
 *
 *       Filename: dbcomrecord.hpp
 *        Created: 07/28/2017 23:03:43
 *    Description: split from dbcom.hpp
 *                 don't include dbcomid.hpp in this header file
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
#include <bit>
#include <cstdint>
#include "dbcomid.hpp"
#include "maprecord.hpp"
#include "buffrecord.hpp"
#include "itemrecord.hpp"
#include "magicrecord.hpp"
#include "monsterrecord.hpp"

template<typename T, uint32_t N> constexpr const T &DBCOM_REFHELPER(const T (&itemList)[N], uint32_t id)
{
    return (id < N) ? itemList[id] : itemList[0];
}

constexpr const ItemRecord           &DBCOM_ITEMRECORD          (uint32_t id) { return DBCOM_REFHELPER(_inn_ItemRecordList,           id); }
constexpr const MagicRecord          &DBCOM_MAGICRECORD         (uint32_t id) { return DBCOM_REFHELPER(_inn_MagicRecordList,          id); }
constexpr const MonsterRecord        &DBCOM_MONSTERRECORD       (uint32_t id) { return DBCOM_REFHELPER(_inn_MonsterRecordList,        id); }
constexpr const MapRecord            &DBCOM_MAPRECORD           (uint32_t id) { return DBCOM_REFHELPER(_inn_MapRecordList,            id); }
constexpr const BuffRecord           &DBCOM_BUFFRECORD          (uint32_t id) { return DBCOM_REFHELPER(_inn_BuffRecordList,           id); }
constexpr const BuffActRecord        &DBCOM_BUFFACTRECORD       (uint32_t id) { return DBCOM_REFHELPER(_inn_BuffActRecordList,        id); }
constexpr const AttackModifierRecord &DBCOM_ATTACKMODIFIERRECORD(uint32_t id) { return DBCOM_REFHELPER(_inn_AttackModifierRecordList, id); }
constexpr const SpellModifierRecord  &DBCOM_SPELLMODIFIERRECORD (uint32_t id) { return DBCOM_REFHELPER(_inn_SpellModifierRecordList,  id); }

constexpr const ItemRecord           &DBCOM_ITEMRECORD          (const char8_t *name) { return DBCOM_ITEMRECORD          (DBCOM_ITEMID          (name)); }
constexpr const MagicRecord          &DBCOM_MAGICRECORD         (const char8_t *name) { return DBCOM_MAGICRECORD         (DBCOM_MAGICID         (name)); }
constexpr const MonsterRecord        &DBCOM_MONSTERRECORD       (const char8_t *name) { return DBCOM_MONSTERRECORD       (DBCOM_MONSTERID       (name)); }
constexpr const MapRecord            &DBCOM_MAPRECORD           (const char8_t *name) { return DBCOM_MAPRECORD           (DBCOM_MAPID           (name)); }
constexpr const BuffRecord           &DBCOM_BUFFRECORD          (const char8_t *name) { return DBCOM_BUFFRECORD          (DBCOM_BUFFID          (name)); }
constexpr const BuffActRecord        &DBCOM_BUFFACTRECORD       (const char8_t *name) { return DBCOM_BUFFACTRECORD       (DBCOM_BUFFACTID       (name)); }
constexpr const AttackModifierRecord &DBCOM_ATTACKMODIFIERRECORD(const char8_t *name) { return DBCOM_ATTACKMODIFIERRECORD(DBCOM_ATTACKMODIFIERID(name)); }
constexpr const SpellModifierRecord  &DBCOM_SPELLMODIFIERRECORD (const char8_t *name) { return DBCOM_SPELLMODIFIERRECORD (DBCOM_SPELLMODIFIERID (name)); }

bool getClothGender(uint32_t);

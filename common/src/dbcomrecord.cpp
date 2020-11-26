/*
 * =====================================================================================
 *
 *       Filename: dbcomrecord.cpp
 *        Created: 07/30/2017 02:01:02
 *    Description: split from dbcom.hpp
 *                 here we include dbcomid.hpp means we have a copy of the
 *                 constexpr _Inn_XXXX[] declared in dbcomid.hpp
 *
 *                 then all reference from current uint(.cpp file) will use
 *                 the same copy of the _Inn_XXXX
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

#include "dbcomid.hpp"
#include "itemrecord.hpp"
#include "monsterrecord.hpp"

template<typename T, size_t N> const T &DBCOM_REFHELPER(const T (&itemList)[N], uint32_t id)
{
    return (id < (uint32_t)(N)) ? itemList[id] : itemList[0];
}

const ItemRecord    &DBCOM_ITEMRECORD   (uint32_t id) { return DBCOM_REFHELPER(_inn_ItemRecordList,    id); }
const MagicRecord   &DBCOM_MAGICRECORD  (uint32_t id) { return DBCOM_REFHELPER(_inn_MagicRecordList,   id); }
const MonsterRecord &DBCOM_MONSTERRECORD(uint32_t id) { return DBCOM_REFHELPER(_inn_MonsterRecordList, id); }
const MapRecord     &DBCOM_MAPRECORD    (uint32_t id) { return DBCOM_REFHELPER(_inn_MapRecordList,     id); }
const NPCRecord     &DBCOM_NPCRECORD    (uint32_t id) { return DBCOM_REFHELPER(_inn_NPCRecordList,     id); }

const ItemRecord    &DBCOM_ITEMRECORD   (const char8_t *name) { return DBCOM_ITEMRECORD   (DBCOM_ITEMID   (name)); }
const MagicRecord   &DBCOM_MAGICRECORD  (const char8_t *name) { return DBCOM_MAGICRECORD  (DBCOM_MAGICID  (name)); }
const MonsterRecord &DBCOM_MONSTERRECORD(const char8_t *name) { return DBCOM_MONSTERRECORD(DBCOM_MONSTERID(name)); }
const MapRecord     &DBCOM_MAPRECORD    (const char8_t *name) { return DBCOM_MAPRECORD    (DBCOM_MAPID    (name)); }
const NPCRecord     &DBCOM_NPCRECORD    (const char8_t *name) { return DBCOM_NPCRECORD    (DBCOM_NPCID    (name)); }

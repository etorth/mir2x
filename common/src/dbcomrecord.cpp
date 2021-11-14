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

#include <string_view>
#include "totype.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "dbcomrecord.hpp"

const static struct ItemRecordAssertor
{
    ItemRecordAssertor()
    {
        fflassert(!DBCOM_ITEMRECORD(nullptr));
        for(uint32_t i = 1; i < DBCOM_ITEMENDID(); ++i){
            const auto &ir = DBCOM_ITEMRECORD(i);
            fflassert(ir);

            for(uint32_t j = i + 1; j < DBCOM_ITEMENDID(); ++j){
                const auto &next_ir = DBCOM_ITEMRECORD(j);
                fflassert(next_ir);
                fflassert(!next_ir.isItem(ir.name));
            }
        }
    }
}
s_itemRecordAssertor {};

const static struct BuffActRecordAssertor
{
    BuffActRecordAssertor()
    {
        fflassert(!DBCOM_BUFFACTRECORD(nullptr));
        for(uint32_t i = 1; i < DBCOM_BUFFACTENDID(); ++i){
            const auto &bar = DBCOM_BUFFACTRECORD(i);
            fflassert(bar);

            for(uint32_t j = i + 1; j < DBCOM_BUFFACTENDID(); ++j){
                const auto &next_bar = DBCOM_BUFFACTRECORD(j);
                fflassert(next_bar);
                fflassert(!next_bar.isBuffAct(bar.name));
            }
        }
    }
}
s_buffActRecordAssertor {};

template<typename T, size_t N> const T &DBCOM_REFHELPER(const T (&itemList)[N], uint32_t id)
{
    return (id < to_u32(N)) ? itemList[id] : itemList[0];
}

const ItemRecord           &DBCOM_ITEMRECORD          (uint32_t id) { return DBCOM_REFHELPER(_inn_ItemRecordList,           id); }
const MagicRecord          &DBCOM_MAGICRECORD         (uint32_t id) { return DBCOM_REFHELPER(_inn_MagicRecordList,          id); }
const MonsterRecord        &DBCOM_MONSTERRECORD       (uint32_t id) { return DBCOM_REFHELPER(_inn_MonsterRecordList,        id); }
const MapRecord            &DBCOM_MAPRECORD           (uint32_t id) { return DBCOM_REFHELPER(_inn_MapRecordList,            id); }
const BuffRecord           &DBCOM_BUFFRECORD          (uint32_t id) { return DBCOM_REFHELPER(_inn_BuffRecordList,           id); }
const BuffActRecord        &DBCOM_BUFFACTRECORD       (uint32_t id) { return DBCOM_REFHELPER(_inn_BuffActRecordList,        id); }
const AttackModifierRecord &DBCOM_ATTACKMODIFIERRECORD(uint32_t id) { return DBCOM_REFHELPER(_inn_AttackModifierRecordList, id); }
const SpellModifierRecord  &DBCOM_SPELLMODIFIERRECORD (uint32_t id) { return DBCOM_REFHELPER(_inn_SpellModifierRecordList,  id); }

const ItemRecord           &DBCOM_ITEMRECORD          (const char8_t *name) { return DBCOM_ITEMRECORD          (DBCOM_ITEMID          (name)); }
const MagicRecord          &DBCOM_MAGICRECORD         (const char8_t *name) { return DBCOM_MAGICRECORD         (DBCOM_MAGICID         (name)); }
const MonsterRecord        &DBCOM_MONSTERRECORD       (const char8_t *name) { return DBCOM_MONSTERRECORD       (DBCOM_MONSTERID       (name)); }
const MapRecord            &DBCOM_MAPRECORD           (const char8_t *name) { return DBCOM_MAPRECORD           (DBCOM_MAPID           (name)); }
const BuffRecord           &DBCOM_BUFFRECORD          (const char8_t *name) { return DBCOM_BUFFRECORD          (DBCOM_BUFFID          (name)); }
const BuffActRecord        &DBCOM_BUFFACTRECORD       (const char8_t *name) { return DBCOM_BUFFACTRECORD       (DBCOM_BUFFACTID       (name)); }
const AttackModifierRecord &DBCOM_ATTACKMODIFIERRECORD(const char8_t *name) { return DBCOM_ATTACKMODIFIERRECORD(DBCOM_ATTACKMODIFIERID(name)); }
const SpellModifierRecord  &DBCOM_SPELLMODIFIERRECORD (const char8_t *name) { return DBCOM_SPELLMODIFIERRECORD (DBCOM_SPELLMODIFIERID (name)); }

bool getClothGender(uint32_t itemID)
{
    if(const auto &ir = DBCOM_ITEMRECORD(itemID); ir && (to_u8sv(ir.type) == u8"衣服")){
        if(to_u8sv(ir.name).find(u8"（男）") != std::u8string_view::npos){
            return true;
        }
        else if(to_u8sv(ir.name).find(u8"（女）") != std::u8string_view::npos){
            return false;
        }
    }
    throw fflerror("invalid argument: itemID = %llu", to_llu(itemID));
}

/*
 * =====================================================================================
 *
 *       Filename: dbcomid.hpp
 *        Created: 07/28/2017 23:03:43
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
#include "maprecord.hpp"
#include "itemrecord.hpp"
#include "magicrecord.hpp"
#include "monsterrecord.hpp"

namespace
{
    // to support use following statement in switch/case
    //
    //      switch(nMonsterID){
    //          case DBCOM_MONSTERID("鹿"):
    //          case DBCOM_MONSTERID("鸡"):
    //          case DBCOM_MONSTERID("狼"):
    //          default:
    //      }
    //
    // this is very useful since
    // 1. don't need to think about all monster english name
    // 2. don't need to assign monster ID to each type, just automatically generate one by .inc file

    constexpr ItemRecord _inn_ItemRecordList []
    {
        #include "itemrecord.inc"
    };

    constexpr MonsterRecord _inn_MonsterRecordList []
    {
        #include "monsterrecord.inc"
    };

    constexpr MagicRecord _inn_MagicRecordList []
    {
        #include "magicrecord.inc"
    };

    constexpr MapRecord _inn_MapRecordList []
    {
        #include "maprecord.inc"
    };
}

template<typename T, size_t N> constexpr uint32_t DBCOM_IDHELPER(const T (&itemList)[N], const char8_t *name)
{
    if(name){
        for(uint32_t i = 0; i < N; ++i){
            if(std::u8string_view(itemList[i].name) == name){
                return i;
            }
        }
    }
    return 0;
}

constexpr uint32_t DBCOM_ITEMID   (const char8_t *name) { return DBCOM_IDHELPER(_inn_ItemRecordList,    name); }
constexpr uint32_t DBCOM_MONSTERID(const char8_t *name) { return DBCOM_IDHELPER(_inn_MonsterRecordList, name); }
constexpr uint32_t DBCOM_MAGICID  (const char8_t *name) { return DBCOM_IDHELPER(_inn_MagicRecordList,   name); }
constexpr uint32_t DBCOM_MAPID    (const char8_t *name) { return DBCOM_IDHELPER(_inn_MapRecordList,     name); }

/*
 * =====================================================================================
 *
 *       Filename: dbcom.hpp
 *        Created: 07/28/2017 23:03:43
 *  Last Modified: 07/30/2017 01:56:31
 *
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
#include "itemrecord.hpp"
#include "monsterrecord.hpp"

namespace
{
    constexpr bool _Inn_CompareUTF8(const char *szStr1, const char *szStr2)
    {
        if(szStr1 && szStr2){
            while(*szStr1 == *szStr2){
                if(*szStr1){
                    ++szStr1;
                    ++szStr2;
                }else{
                    return true;
                }
            }
        }
        return false;
    }

    // TODO
    // should be very very very careful here
    // this is an constant variable in header file
    // for each unit (.cpp file) include it, there is a identical copy of it
    // so between different .cpp file, never aceess pointer of this _Inn_XXXXX[]

    // why I introduce such a mess?
    // I want use following statement in switch/case
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
    //
    // but this need DBCOM_MONSTERID() to be ``constexpr"
    // and a constexpr function implies it's an inline function

    constexpr ItemRecord _Inn_ItemRecordList []
    {
        #include "itemrecord.inc"
    };

    constexpr MonsterRecord _Inn_MonsterRecordList []
    {
        #include "monsterrecord.inc"
    };
}

// constexpr function to map utf-8 string to item record id
// when use it in compile time never warry about its performance
//
// if fallback to runtime:
//
//      auto nID = DBCOM_ITEMID(szName);
//
// this would cause line search for all item record in itemrecord.inc
// would be better if we can create a constexpr hashmap based on the data
// check following repo:
//
//      https://github.com/benjibc/constexpr_hash_map.git
//
// but we don't need it currently, principle:
// transfer ID between client and server but not string name
// so we won't need to retrieve ID by a given a string variable name
// only use this function with literal string to get ID since ID is not fixed (but unique)
constexpr uint32_t DBCOM_ITEMID(const char *szName)
{
    if(szName){
        for(size_t nIndex = 0; nIndex < sizeof(_Inn_ItemRecordList) / sizeof(_Inn_ItemRecordList[0]); ++nIndex){
            if(_Inn_CompareUTF8(szName, _Inn_ItemRecordList[nIndex].Name)){
                return (uint32_t)(nIndex);
            }
        }
    }
    return 0;
}

constexpr uint32_t DBCOM_MONSTERID(const char *szName)
{
    if(szName){
        for(size_t nIndex = 0; nIndex < sizeof(_Inn_MonsterRecordList) / sizeof(_Inn_MonsterRecordList[0]); ++nIndex){
            if(_Inn_CompareUTF8(szName, _Inn_MonsterRecordList[nIndex].Name)){
                return (uint32_t)(nIndex);
            }
        }
    }
    return 0;
}


// TODO
// sometimes I only need following record-retrieving functions
// then inclusion of this file adds un-needed copy for the constexpr arrays

// I may split this file as
// dbcom_id.hpp         // for DBCOM_XXXXID()
// dbcom_record.hpp     // for DBCOM_XXXXRECORD()

const ItemRecord &DBCOM_ITEMRECORD(uint32_t);
const ItemRecord &DBCOM_ITEMRECORD(const char *);

const MonsterRecord &DBCOM_MONSTERRECORD(uint32_t);
const MonsterRecord &DBCOM_MONSTERRECORD(const char *);

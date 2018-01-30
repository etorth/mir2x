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

const ItemRecord &DBCOM_ITEMRECORD(uint32_t nID)
{
    if(true
            && nID > 0
            && nID < sizeof(_Inn_ItemRecordList) / sizeof(_Inn_ItemRecordList[0])){
        return _Inn_ItemRecordList[nID];
    }
    return _Inn_ItemRecordList[0];
}

const ItemRecord &DBCOM_ITEMRECORD(const char *szName)
{
    return DBCOM_ITEMRECORD(DBCOM_ITEMID(szName));
}

const MagicRecord &DBCOM_MAGICRECORD(uint32_t nID)
{
    if(true
            && nID > 0
            && nID < sizeof(_Inn_MagicRecordList) / sizeof(_Inn_MagicRecordList[0])){
        return _Inn_MagicRecordList[nID];
    }
    return _Inn_MagicRecordList[0];
}

const MagicRecord &DBCOM_MAGICRECORD(const char *szName)
{
    return DBCOM_MAGICRECORD(DBCOM_MAGICID(szName));
}

const MonsterRecord &DBCOM_MONSTERRECORD(uint32_t nID)
{
    if(true
            && nID > 0
            && nID < sizeof(_Inn_MonsterRecordList) / sizeof(_Inn_MonsterRecordList[0])){
        return _Inn_MonsterRecordList[nID];
    }
    return _Inn_MonsterRecordList[0];
}

const MonsterRecord &DBCOM_MONSTERRECORD(const char *szName)
{
    return DBCOM_MONSTERRECORD(DBCOM_MONSTERID(szName));
}

const MapRecord &DBCOM_MAPRECORD(uint32_t nID)
{
    if(true
            && nID > 0
            && nID < sizeof(_Inn_MapRecordList) / sizeof(_Inn_MapRecordList[0])){
        return _Inn_MapRecordList[nID];
    }
    return _Inn_MapRecordList[0];
}

const MapRecord &DBCOM_MAPRECORD(const char *szName)
{
    return DBCOM_MAPRECORD(DBCOM_MAPID(szName));
}

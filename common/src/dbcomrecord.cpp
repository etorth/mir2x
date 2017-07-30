/*
 * =====================================================================================
 *
 *       Filename: dbcomrecord.cpp
 *        Created: 07/30/2017 02:01:02
 *  Last Modified: 07/30/2017 02:06:13
 *
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

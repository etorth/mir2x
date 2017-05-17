/*
 * =====================================================================================
 *
 *       Filename: dbconst.cpp
 *        Created: 05/12/2017 17:58:02
 *  Last Modified: 05/16/2017 18:22:34
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

#include <unordered_map>

#include "dbconst.hpp"
#include "protocoldef.hpp"
#include "monsterrecord.hpp"

const DCRecord &DB_DCRECORD(uint32_t nDC)
{
    static std::unordered_map<uint32_t, DCRecord> stDCRecord
    {
        {           0, {           0, 0, 0, u8"", u8""}},
        {DC_PHY_PLAIN, {DC_PHY_PLAIN, 0, 0, u8"", u8""}},
    };
    return stDCRecord.at((stDCRecord.find(nDC) != stDCRecord.end()) ? nDC : 0);
}

const MonsterRecord &DB_MONSTERRECORD(uint32_t nMonsterID)
{
    static std::unordered_map<uint32_t, MonsterRecord> stMonsterRecord
    {
        // ----+----+--------+-------+--------+---------+---------+----+----+-----+-----+---------+--------+-------+---------+--------+--------+--------+-----------+----+-------+--------+----+-------+----------+----------+-----------+------------+------------+--------------+------------------+--------------+-------------------+
        //     | ID | LookID | Level | Undead | Tameble | CoolEye | HP | MP | Hit | Exp | ACPlain | ACFire | ACIce | ACLight | ACWind | ACHoly | ACDark | ACPhantom | DC | DCMax | MCType | MC | MCMax | WalkStep | WalkWait | WalkSpeed | AttackMode | AttackWait | AttackEffect | DCList           | Name         | Description       |
        {   0, {  0 , 0X0000 ,   0   ,    0   ,    0    ,    0    ,  0 ,  0 ,  0  ,  0  ,   0     ,    0   ,   0   ,    0    ,    0   ,    0   ,    0   ,     0     ,  0 ,   0   ,    0   ,  0 ,   0   ,     0    ,      0   ,     0     ,      0     ,      0     ,      0       , {}               , u8""         , u8""             }},
        {   1, {  1 , 0X0015 ,   1   ,    0   ,    1    ,    0    , 10 , 10 , 10  , 10  ,   1     ,    0   ,   0   ,    0    ,    0   ,    0   ,    0   ,     0     ,  1 ,   2   ,    0   ,  0 ,   0   ,     1    ,   1000   ,     1     ,      0     ,   1000     ,      0       , {DC_PHY_PLAIN}   , u8"鹿"       , u8""             }},
        {  10, { 10 , 0X009F ,   1   ,    0   ,    1    ,    0    , 10 , 10 , 10  , 10  ,   1     ,    0   ,   0   ,    0    ,    0   ,    0   ,    0   ,     0     ,  1 ,   2   ,    0   ,  0 ,   0   ,     1    ,   1000   ,     1     ,      0     ,   1000     ,      0       , {DC_PHY_PLAIN}   , u8"天狮辟邪" , u8""             }},
    };

    return stMonsterRecord.at((stMonsterRecord.find(nMonsterID) != stMonsterRecord.end()) ? nMonsterID : 0);
}

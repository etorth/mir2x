/*
 * =====================================================================================
 *
 *       Filename: dropitemconfig.cpp
 *        Created: 07/30/2017 00:12:33
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

#include <cstdint>
#include <cinttypes>

#include "dbcomid.hpp"
#include "monoserver.hpp"
#include "dropitemconfig.hpp"

extern MonoServer *g_monoServer;

DropItemConfig::operator bool() const
{
    return true
        && DBCOM_MONSTERID(monsterName)
        && DBCOM_ITEMID(itemName)

        && group     >= 0
        && probRecip >= 1

        && repeat >= 1
        && value  >= 1
        ;
}

void DropItemConfig::print() const
{
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::monsterName = %s", monsterName);
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::itemName    = %s", itemName   );
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::group       = %d", group      );
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::probRecip   = %d", probRecip  );
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::repeat      = %d", repeat     );
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::value       = %d", value      );
}

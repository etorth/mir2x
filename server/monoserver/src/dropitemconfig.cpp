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

DropItemConfig::operator bool() const
{
    return true
        && DBCOM_MONSTERID(MonsterName)
        && DBCOM_ITEMID(ItemName)

        && Group     >= 0
        && ProbRecip >= 1

        && Repeat >= 1
        && Value  >= 1
        ;
}

void DropItemConfig::Print() const
{
    extern MonoServer *g_monoServer;
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::0X%0*" PRIXPTR "::MonsterName = %s", (int)(2 * sizeof(this)), (uintptr_t)(this), MonsterName);
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::0X%0*" PRIXPTR "::ItemName    = %s", (int)(2 * sizeof(this)), (uintptr_t)(this), ItemName   );
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::0X%0*" PRIXPTR "::Group       = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Group      );
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::0X%0*" PRIXPTR "::ProbRecip   = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), ProbRecip  );
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::0X%0*" PRIXPTR "::Repeat      = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Repeat     );
    g_monoServer->addLog(LOGTYPE_INFO, "DropItemConfig::0X%0*" PRIXPTR "::Value       = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Value      );
}

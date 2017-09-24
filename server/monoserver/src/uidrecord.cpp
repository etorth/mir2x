/*
 * =====================================================================================
 *
 *       Filename: uidrecord.cpp
 *        Created: 05/02/2017 16:11:11
 *  Last Modified: 09/23/2017 22:26:48
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

#include <cinttypes>
#include "uidrecord.hpp"
#include "monoserver.hpp"

UIDRecord::UIDRecord(uint32_t nUID,
        const InvarData &rstDesp,
        const Theron::Address &rstAddress,
        const std::vector<ServerObject::ClassCodeName> &rstClassEntry)
    : UID(nUID)
    , Desp(rstDesp)
    , Address(rstAddress)
    , ClassEntry(rstClassEntry)
{
    if(false
            || UID == 0
            || ClassEntry.empty()){
        // we need to return an empty record to caller
        // allow this since when retrieving UID failed form UIDArray
    }
}

void UIDRecord::Print() const
{
    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_INFO, "UIDRecord::UID                  = %" PRIu32, UID);
    for(size_t nIndex= 0; nIndex < ClassEntry.size(); ++nIndex){
        g_MonoServer->AddLog(LOGTYPE_INFO, "UIDRecord::ClassEntry[%d]::Code = %llu", (int)(nIndex), (unsigned long long)(ClassEntry[nIndex].Code));
        g_MonoServer->AddLog(LOGTYPE_INFO, "UIDRecord::ClassEntry[%d]::Name = %s",   (int)(nIndex), ClassEntry[nIndex].Name.c_str());
    }
}

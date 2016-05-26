/*
 * =====================================================================================
 *
 *       Filename: servicecorenet.cpp
 *        Created: 05/20/2016 17:09:13
 *  Last Modified: 05/25/2016 18:34:02
 *
 *    Description: interaction btw SessionHub and ServiceCore
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
#include "dbpod.hpp"
#include "threadpn.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"

void ServiceCore::Net_CM_Login(uint32_t nSessionID, uint8_t, const uint8_t *pData, size_t)
{
    // message structure:  IP\0Port\0ID\0PWD\0
    // copy it out since pData is temperal
    std::string szIP   = (char *)pData;    pData += std::strlen((char *)pData);
    std::string szPort = (char *)pData;    pData += std::strlen((char *)pData);
    std::string szID   = (char *)pData;    pData += std::strlen((char *)pData);
    std::string szPWD  = (char *)pData; // pData += std::strlen((char *)pData);

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_INFO,
            "Login requested from (%s:%s)", szIP.c_str(), szPort.c_str());

    // don't block ServiceCore too much, so we put rest of it 
    // in the thread pool since it's db query and slow
    //
    // here we put pSession, but how about this session has been killed
    // when this lambda invoked
    auto fnDBOperation = [nSessionID, stSCAddr = GetAddress(), szID, szPWD](){
        extern DBPodN *g_DBPodN;
        extern MonoServer *g_MonoServer;

        auto pDBHDR = g_DBPodN->CreateDBHDR();

        if(!pDBHDR->Execute("select fld_id from tbl_account where "
                    "fld_account = '%s' and fld_password = '%s'", szID.c_str(), szPWD.c_str())){
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "SQL ERROR: (%d: %s)", pDBHDR->ErrorID(), pDBHDR->ErrorInfo());
            SyncDriver().Forward({SM_LOGINFAIL, nSessionID}, stSCAddr);
            return;
        }

        if(pDBHDR->RowCount() < 1){
            g_MonoServer->AddLog(LOGTYPE_INFO,
                    "can't find account: (%s:%s)", szID.c_str(), szPWD.c_str());
            SyncDriver().Forward({SM_LOGINFAIL, nSessionID}, stSCAddr);
            return;
        }

        pDBHDR->Fetch();
        // you can put another lambda here and put it in g_TaskHub
        // but doesn't make sense since this function is already slow
        int nID = std::atoi(pDBHDR->Get("fld_id"));
        if(!pDBHDR->Execute("select * from mir2x.tbl_guid where fld_id = %d", nID)){
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "SQL ERROR: (%d: %s)", pDBHDR->ErrorID(), pDBHDR->ErrorInfo());
            SyncDriver().Forward({SM_LOGINFAIL, nSessionID}, stSCAddr);
            return;
        }

        if(pDBHDR->RowCount() < 1){
            g_MonoServer->AddLog(LOGTYPE_INFO,
                    "no guid created for this account: (%s:%s)", szID.c_str(), szPWD.c_str());
            SyncDriver().Forward({SM_LOGINFAIL, nSessionID}, stSCAddr);
            return;
        }

        // ok now we found the record
        // currently only handle the first record

        AMLoginQueryDB stAMLQDB;

        stAMLQDB.GUID  = std::atoi(pDBHDR->Get("fld_guid"));
        stAMLQDB.MapID = std::atoi(pDBHDR->Get("fld_mapid"));
        stAMLQDB.MapX  = std::atoi(pDBHDR->Get("fld_mapx"));
        stAMLQDB.MapY  = std::atoi(pDBHDR->Get("fld_mapy"));

        stAMLQDB.Level  = std::atoi(pDBHDR->Get("fld_level"));
        stAMLQDB.Job    = std::atoi(pDBHDR->Get("fld_level"));
        stAMLQDB.Direction = std::atoi(pDBHDR->Get("fld_level"));

        std::strncpy(stAMLQDB.Name, pDBHDR->Get("fld_guid"), sizeof(stAMLQDB.Name));
        SyncDriver().Forward({MPK_LOGINQUERYDB, stAMLQDB}, stSCAddr);
    };

    extern ThreadPN *g_ThreadPN;
    g_ThreadPN->Add(fnDBOperation);
}

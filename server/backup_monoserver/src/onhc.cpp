/*
 * =====================================================================================
 *
 *       Filename: onhc.cpp
 *        Created: 02/28/2016 01:37:19
 *  Last Modified: 04/19/2016 23:40:26
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

#include <cstdint>
#include "taskhub.hpp"
#include "message.hpp"
#include "session.hpp"
#include "monoserver.hpp"

void MonoServer::OnPing(Session *pSession)
{
    auto fnProcessPing = [this, pSession](uint8_t *pData, size_t){
        pSession->Send(SM_PING, pData, 4);
    };

    pSession->Read(4, fnProcessPing);
}

void MonoServer::OnLogin(Session *pSession)
{
    // TODO
    // pSession may become invalid when CB actually invoked
    int nSessionID = pSession->ID();

    auto fnProcessLogin = [this, nSessionID](uint8_t *pData, int){
        auto pSession = m_SessionHub->Validate(nSessionID);
        if(!pSession){
            AddLog(LOGTYPE_INFO, "Session %d deleted, login request ignored", nSessionID);
            return;
        }

        AddLog(LOGTYPE_INFO, "Login requested from (%s:%d)", pSession->IP(), pSession->Port());
        auto stCMLogin = *((CMLogin *)pData);

        auto fnDBOperation = [this, stCMLogin, nSessionID](){
            auto pSession = m_SessionHub->Validate(nSessionID);
            if(!pSession){
                AddLog(LOGTYPE_INFO, "Session %d deleted, db query ignored", nSessionID);
                pSession->Send(SM_LOGINFAIL);
                return;
            }

            auto pRecord = m_DBConnection->CreateDBRecord();

            const char *pID  = stCMLogin.ID;
            const char *pPWD = stCMLogin.Password;

            if(!pRecord->Execute("select fld_id from tbl_account "
                        "where fld_account = '%s' and fld_password = '%s'", pID, pPWD)){
                AddLog(2, "SQL ERROR: (%d: %s)", pRecord->ErrorID(), pRecord->ErrorInfo());
                pSession->Send(SM_LOGINFAIL);
                return;
            }

            if(pRecord->RowCount() < 1){
                AddLog(LOGTYPE_INFO, "can't find account: (%s:%s)", pID, pPWD);
                pSession->Send(SM_LOGINFAIL);
                return;
            }

            pRecord->Fetch();
            // you can put another lambda here and put it in g_TaskHub
            // but doesn't make sense since this function is already slow
            int nID = std::atoi(pRecord->Get("fld_id"));
            if(!pRecord->Execute("select * from mir2x.tbl_guid where fld_id = %d", nID)){
                AddLog(2, "SQL ERROR: (%d: %s)", pRecord->ErrorID(), pRecord->ErrorInfo());
                pSession->Send(SM_LOGINFAIL);
                return;
            }

            if(pRecord->RowCount() < 1){
                AddLog(LOGTYPE_INFO, "no guid created for this account: (%s:%s)", pID, pPWD);
                pSession->Send(SM_LOGINFAIL);
                return;
            }

            SMLoginOK *pSMLoginOK = new SMLoginOK();
            // currently only handle the first record
            pRecord->Fetch();
            // std::strncpy(pSMLoginOK->Name, 
            //         pRecord->Get("fld_name"), sizeof(pSMLoginOK->Name));

            // std::strncpy(pSMLoginOK->MapName,
            //         pRecord->Get("fld_map"), sizeof(pSMLoginOK->MapName));

            // TODO
            // tmp hack
            std::strncpy(pSMLoginOK->Name,    "test",     sizeof(pSMLoginOK->Name));
            std::strncpy(pSMLoginOK->MapName, "DESC.BIN", sizeof(pSMLoginOK->MapName));

            pSMLoginOK->GUID = std::atoi(pRecord->Get("fld_guid"));
            // pSMLoginOK->SID       = std::atoi(pRecord->Get("fld_sid"));
            // pSMLoginOK->Level     = std::atoi(pRecord->Get("fld_level"));
            // pSMLoginOK->MapX      = std::atoi(pRecord->Get("fld_x"));
            // pSMLoginOK->MapY      = std::atoi(pRecord->Get("fld_y"));
            // pSMLoginOK->Direction = std::atoi(pRecord->Get("fld_direction"));

            // blocking
            // if(PlayerLogin(stSMLoginOK)){
            //     pSession->Send(SM_LOGINOK, stSMLoginOK);
            // }

            if(!AddPlayer(nSessionID, pSMLoginOK->GUID)){
                AddLog(LOGTYPE_INFO, "Add player failed: (%s:%s:%d:%s:%d)",
                        pID, pPWD, nID, pSMLoginOK->Name, pSMLoginOK->GUID);
                pSession->Send(SM_LOGINFAIL);
                return;
            }

            AddLog(LOGTYPE_INFO, "Login succeed: (%s:%s:%d:%s:%d)",
                    pID, pPWD, nID, pSMLoginOK->Name, pSMLoginOK->GUID);
            pSession->Send(SM_LOGINOK, *pSMLoginOK, [pSMLoginOK](){ delete pSMLoginOK; });
        };

        extern TaskHub *g_TaskHub;
        g_TaskHub->Add(fnDBOperation);
    };

    pSession->Read(sizeof(CMLogin), fnProcessLogin);
}

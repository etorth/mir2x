/*
 * =====================================================================================
 *
 *       Filename: onhc.cpp
 *        Created: 02/28/2016 01:37:19
 *  Last Modified: 04/06/2016 02:33:47
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
        auto pSession = m_SessionIO->Validate(nSessionID);
        if(!pSession){
            AddLog(LOGTYPE_INFO, "Session %d deleted, login request ignored", nSessionID);
            return;
        }

        AddLog(LOGTYPE_INFO, "Login requested from (%s:%d)", pSession->IP(), pSession->Port());
        auto stCMLogin = *((CMLogin *)pData);

        auto fnDBOperation = [this, stCMLogin, nSessionID](){
            auto pSession = m_SessionIO->Validate(nSessionID);
            if(!pSession){
                AddLog(LOGTYPE_INFO, "Session %d deleted, db query ignored", nSessionID);
                return;
            }

            auto pRecord = m_DBConnection->CreateDBRecord();

            const char *pID  = stCMLogin.ID;
            const char *pPWD = stCMLogin.Password;

            if(!pRecord->Execute("select fld_id from tbl_account "
                        "where fld_account = '%s' and fld_password = '%s'", pID, pPWD)){
                AddLog(2, "SQL ERROR: (%d: %s)", pRecord->ErrorID(), pRecord->ErrorInfo());
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
                return;
            }

            if(pRecord->RowCount() < 1){
                AddLog(LOGTYPE_INFO, "no guid created for this account: (%s:%s)", pID, pPWD);
                pSession->Send(SM_LOGINFAIL);
                return;
            }

            SMLoginOK stSMLoginOK;
            // currently only handle the first record
            pRecord->Fetch();
            std::strncpy(stSMLoginOK.Name, pRecord->Get("fld_name"), sizeof(stSMLoginOK.Name));
            // std::strncpy(stSMLoginOK.MapName,
            //         pRecord->Get("fld_map"), sizeof(stSMLoginOK.MapName));

            stSMLoginOK.GUID = std::atoi(pRecord->Get("fld_guid"));
            // stSMLoginOK.SID       = std::atoi(pRecord->Get("fld_sid"));
            // stSMLoginOK.Level     = std::atoi(pRecord->Get("fld_level"));
            // stSMLoginOK.MapX      = std::atoi(pRecord->Get("fld_x"));
            // stSMLoginOK.MapY      = std::atoi(pRecord->Get("fld_y"));
            // stSMLoginOK.Direction = std::atoi(pRecord->Get("fld_direction"));

            // blocking
            // if(PlayerLogin(stSMLoginOK)){
            //     pSession->Send(SM_LOGINOK, stSMLoginOK);
            // }
            pSession->Send(SM_LOGINOK, stSMLoginOK);
            AddLog(LOGTYPE_INFO, "Login succeed: (%s:%s:%d:%s:%d)",
                    pID, pPWD, nID, stSMLoginOK.Name, stSMLoginOK.GUID);
            pSession->Send(SM_LOGINFAIL);
        };

        extern TaskHub *g_TaskHub;
        g_TaskHub->Add(fnDBOperation);
    };

    pSession->Read(sizeof(CMLogin), fnProcessLogin);
}

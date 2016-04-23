/*
 * =====================================================================================
 *
 *       Filename: onhc.cpp
 *        Created: 02/28/2016 01:37:19
 *  Last Modified: 04/23/2016 02:12:28
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

// when calling this function we need to forward message to service core
// then the core create player if login succeeds
void MonoServer::OnLogin(Session *pSession)
{
    // pSession will keep valid always, until we delete it manully, so
    //      1. we don't need to check its validation
    //      2. in view of session, read data is processed one by
    //         one in one single thread, this means whening using
    //         pSession, we don't concern it could be delete by
    //         other thread
    auto fnProcessLogin = [pSession](uint8_t *pData, size_t){
        AddLog(LOGTYPE_INFO, "Login requested from (%s:%d)", pSession->IP(), pSession->Port());
        auto stCMLogin = *((CMLogin *)pData);

        auto fnDBOperation = [this, stCMLogin, pSession](){
            auto pRecord = m_DBConnection->CreateDBRecord();

            const char *pID  = stCMLogin.ID;
            const char *pPWD = stCMLogin.Password;

            if(!pRecord->Execute("select fld_id from tbl_account "
                        "where fld_account = '%s' and fld_password = '%s'", pID, pPWD)){
                AddLog(LOGTYPE_WARNING,
                        "SQL ERROR: (%d: %s)", pRecord->ErrorID(), pRecord->ErrorInfo());
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
                AddLog(LOGTYPE_WARNING,
                        "SQL ERROR: (%d: %s)", pRecord->ErrorID(), pRecord->ErrorInfo());
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

            AMLogin stAML = {
                .GUID = pSMLoginOK->GUID,
            }

            MessagePack stMPK;
            while(true){ if(!m_Catcher.Pop(stMPK, m_ServiceCoreAddress)){ break; } }

            extern Theron::Framework *g_Framework;
            g_Framework->Send(MessagePack(MPK_LOGIN,
                        stAML), m_Receiver.GetAddress(), m_ServiceCoreAddress);

            if(true
                    && m_Receiver.Wait(1) == 1
                    && m_Catcher.Pop(stMPK, m_ServiceCoreAddress)
                    && stMPK.Type() == MPK_LOGINOK){

                AddLog(LOGTYPE_INFO, "Login succeed: (%s:%s:%d:%s:%d)",
                        pID, pPWD, nID, pSMLoginOK->Name, pSMLoginOK->GUID);
                pSession->Send(SM_LOGINOK, *pSMLoginOK, [pSMLoginOK](){ delete pSMLoginOK; });

                return;
            }

            AddLog(LOGTYPE_INFO, "Add player failed: (%s:%s:%d:%s:%d)",
                    pID, pPWD, nID, pSMLoginOK->Name, pSMLoginOK->GUID);
            pSession->Send(SM_LOGINFAIL);
            return;
        };

        extern ThreadPN *g_ThreadPN;
        g_ThreadPN->Add(fnDBOperation);
    };

    pSession->Read(sizeof(CMLogin), fnProcessLogin);
}

void MonoServer::OnWalk(Session *pSession)
{
    auto fnProcessWalk = [this, pSession](const uint8_t *pData, size_t){
        // 1. get binding player address
        auto stAddr = pSession->Address();
        if(stAddr == Theron::Address::Null()){ return; }

        // 2. just forward the message to binding player
        extern Theron::Framework *g_Framework;
        g_Framework->Send(MessagePack(MPK_FORWARDCM,
                    CM_WALK, pData, sizeof(CMWalk)), m_Receiver.GetAddress(), stAddr);
    };
    pSession->Read(sizeof(CMWalk), fnProcessWalk);
}

void MonoServer::OnForward(uint8_t nMsgHC, Session *pSession)
{
    extern MonoServer *g_MonoServer;
    size_t nSize = g_MonoServer->MessageSize(nMsgHC);

    auto fnProcessForward = [this, pSession, nSize](const uint8_t *pData, size_t){
        // 1. get binding player address
        auto stAddr = pSession->Address();
        if(stAddr == Theron::Address::Null()){ return; }

        // 2. just forward the message to binding player
        extern Theron::Framework *g_Framework;
        g_Framework->Send(MessagePack(MPK_FORWARDCM,
                    nMsgHC, pData, nSize), m_Receiver.GetAddress(), stAddr);
    };

    pSession->Read(nSize, fnProcessForward);
}

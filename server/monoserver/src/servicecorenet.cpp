/*
 * =====================================================================================
 *
 *       Filename: servicecorenet.cpp
 *        Created: 05/20/2016 17:09:13
 *  Last Modified: 05/20/2016 18:11:31
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

ServiceCore::Net_CM_Login(Session *pSession, uint8_t, const uint8_t *pData, size_t nDataLen)
{
    // message structure:  IP\0Port\0ID\0PWD\0
    std::string szIP   = (char *)pData;    pData += std::strlen((char *)pData);
    std::string szPort = (char *)pData;    pData += std::strlen((char *)pData);
    std::string szID   = (char *)pData;    pData += std::strlen((char *)pData);
    std::string szPWD  = (char *)pData; // pData += std::strlen((char *)pData);

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_INFO,
            "Login requested from (%s:%s)", szIP.c_str(), szPort.c_str());

    // don't block ServiceCore too much, so we put rest of it 
    // in the thread pool since it's db query and slow
    auto fnDBOperation = [this, pSession, szID, szPWD](){
        extern MonoServer *g_MonoServer;
        auto pDBHDR = g_MonoServer->CreateDBHDR();

        if(!pDBHDR->Execute("select fld_id from tbl_account where "
                    "fld_account = '%s' and fld_password = '%s'", szID.c_str(), szPWD.c_str())){
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "SQL ERROR: (%d: %s)", pDBHDR->ErrorID(), pDBHDR->ErrorInfo());
            pSession->Forward(SM_LOGINFAIL);
            return;
        }

        if(pDBHDR->RowCount() < 1){
            g_MonoServer->AddLog(LOGTYPE_INFO, "can't find account: (%s:%s)", pID, pPWD);
            pSession->Forward(SM_LOGINFAIL);
            return;
        }

        pDBHDR->Fetch();
        // you can put another lambda here and put it in g_TaskHub
        // but doesn't make sense since this function is already slow
        int nID = std::atoi(pDBHDR->Get("fld_id"));
        if(!pDBHDR->Execute("select * from mir2x.tbl_guid where fld_id = %d", nID)){
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "SQL ERROR: (%d: %s)", pDBHDR->ErrorID(), pDBHDR->ErrorInfo());
            pSession->Forward(SM_LOGINFAIL);
            return;
        }

        if(pDBHDR->RowCount() < 1){
            g_MonoServer->AddLog(LOGTYPE_INFO,
                    "no guid created for this account: (%s:%s)", pID, pPWD);
            pSession->Forward(SM_LOGINFAIL);
            return;
        }

        SMLoginOK *pSMLoginOK = new SMLoginOK();
        // currently only handle the first record
        pDBHDR->Fetch();
        // std::strncpy(pSMLoginOK->Name, 
        //         pDBHDR->Get("fld_name"), sizeof(pSMLoginOK->Name));

        // std::strncpy(pSMLoginOK->MapName,
        //         pDBHDR->Get("fld_map"), sizeof(pSMLoginOK->MapName));

        // TODO
        // tmp hack
        std::strncpy(pSMLoginOK->Name,    "test",     sizeof(pSMLoginOK->Name));
        std::strncpy(pSMLoginOK->MapName, "DESC.BIN", sizeof(pSMLoginOK->MapName));

        pSMLoginOK->GUID = std::atoi(pDBHDR->Get("fld_guid"));
        // pSMLoginOK->SID       = std::atoi(pDBHDR->Get("fld_sid"));
        // pSMLoginOK->Level     = std::atoi(pDBHDR->Get("fld_level"));
        // pSMLoginOK->MapX      = std::atoi(pDBHDR->Get("fld_x"));
        // pSMLoginOK->MapY      = std::atoi(pDBHDR->Get("fld_y"));
        // pSMLoginOK->Direction = std::atoi(pDBHDR->Get("fld_direction"));

        // blocking
        // if(PlayerLogin(stSMLoginOK)){
        //     pSession->Send(SM_LOGINOK, stSMLoginOK);
        // }

        m_MapRMCache

        AMLogin stAML;
        stAML.GUID = pSMLoginOK->GUID;

        MessagePack stMPK;
        if(Send(MessageBuf(MPK_LOGIN, stAML), m_ServiceCoreAddress, &stMPK)){
            switch(stMPK.Type()){
                case MPK_LOGINOK:
                    {
                        g_MonoServer->AddLog(LOGTYPE_INFO,
                                "Login succeed: (%s:%s:%d:%s:%d)",
                                pID, pPWD, nID, pSMLoginOK->Name, pSMLoginOK->GUID);
                        pSession->Forward(SM_LOGINOK, *pSMLoginOK,
                                [pSMLoginOK](){ delete pSMLoginOK; });
                        return;
                    }
                default:
                    {
                        g_MonoServer->AddLog(LOGTYPE_INFO, "Add player failed: (%s:%s:%d:%s:%d)",
                                pID, pPWD, nID, pSMLoginOK->Name, pSMLoginOK->GUID);
                        pSession->Forward(SM_LOGINFAIL);
                        return;
                    }

            }
        }

        g_MonoServer->AddLog(LOGTYPE_INFO, "Send actor message failed");
        pSession->Forward(SM_LOGINFAIL);
        return;
    };

    extern ThreadPN *g_ThreadPN;
    g_ThreadPN->Add(fnDBOperation);
}

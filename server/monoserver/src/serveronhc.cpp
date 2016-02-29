/*
 * =====================================================================================
 *
 *       Filename: serveronhc.cpp
 *        Created: 02/28/2016 01:37:19
 *  Last Modified: 02/29/2016 01:13:03
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

void MonoServer::OnPing(Session *pSession)
{
    auto fnPorcessPing = [this, pSession](uint8_t *pData, size_t){
        std::cout << (uint32_t)(*pData) << std::endl;
        pSession->Send(SM_PING, pData, 4);
    };

    pSession->Read(4, fnProcessPing);
}

bool MonoServer::OnLogin(Session *pSession)
{
    auto fnProcessLogin = [](uint8_t *pData, int){
        Log(0, "Login requested from (%s:%d)", pSession->IP(), pSession->Port());

        auto pCMLogin = (CMLogin *)pData;
        auto pRecord  = m_UserInfoDB->CreateDBRecord();

        if(pRecord->Execute( "select * from userinfo where fld_id = '%s' and fld_pwd = '%s'",
                    pCMLogin->ID, pCMLogin->Password)){
            if(pRecord->RowCount() != 1){
                pSession->Send(SM_LOGINFAIL);
            }else{
                SMLoginSucceed stTmpSM;
                pRecord->Fetch();

                std::strcpy(stTmpSM.CharName, pRecord->Get("fld_name"));
                std::strcpy(stTmpCM.MapName,  pRecord->Get("fld_map"));

                stTmpSM.UID       = std::atoi(pRecord->Get("fld_uid"));
                stTmpSM.SID       = std::atoi(pRecord->Get("fld_sid"));
                stTmpSM.Level     = std::atoi(pRecord->Get("fld_level"));
                stTmpSM.MapX      = std::atoi(pRecord->Get("fld_x"));
                stTmpSM.MapY      = std::atoi(pRecord->Get("fld_y"));
                stTmpSM.Direction = std::atoi(pRecord->Get("fld_direction"));

                // blocking
                if(PlayerLogin(stTmpSM)){
                    pSession->Send(SM_LOGINOK, &stTmpSM, sizeof(stTmpSM));
                }
            }
        }else{
            Log(2, "SQL ERROR: (%d: %s)", pRecord->ErrorID(), pRecord->ErrorInfo());
        }
    };

    pSession->Read(sizeof(CMLogin), fnProcessLogin);
}

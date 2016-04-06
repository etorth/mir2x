/*
 * =====================================================================================
 *
 *       Filename: onhc.cpp
 *        Created: 02/28/2016 01:37:19
 *  Last Modified: 04/04/2016 17:53:42
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
    auto fnProcessLogin = [this, pSession](uint8_t *pData, int){
        AddLog(0, "Login requested from (%s:%d)", pSession->IP(), pSession->Port());

        auto stCMLogin = *((CMLogin *)pData);
        auto fnDBOp = [this, stCMLogin, pSession](){
            auto pRecord  = m_UserInfoDB->CreateDBRecord();

            if(pRecord->Execute( "select * from userinfo where fld_id = '%s' and fld_pwd = '%s'",
                        stCMLogin.ID, stCMLogin.Password)){
                if(pRecord->RowCount() != 1){
                    pSession->Send(SM_LOGINFAIL);
                }else{
                    SMLoginOK stTmpSM;
                    pRecord->Fetch();

                    std::strncpy(stTmpSM.CharName, pRecord->Get("fld_name"), sizeof(stTmpSM.CharName));
                    std::strncpy(stTmpSM.MapName , pRecord->Get("fld_map"),  sizeof(stTmpSM.MapName ));

                    stTmpSM.UID       = std::atoi(pRecord->Get("fld_uid"));
                    stTmpSM.SID       = std::atoi(pRecord->Get("fld_sid"));
                    stTmpSM.Level     = std::atoi(pRecord->Get("fld_level"));
                    stTmpSM.MapX      = std::atoi(pRecord->Get("fld_x"));
                    stTmpSM.MapY      = std::atoi(pRecord->Get("fld_y"));
                    stTmpSM.Direction = std::atoi(pRecord->Get("fld_direction"));

                    // blocking
                    if(PlayerLogin(stTmpSM)){
                        pSession->Send(SM_LOGINOK, stTmpSM);
                    }
                }
            }else{
                AddLog(2, "SQL ERROR: (%d: %s)", pRecord->ErrorID(), pRecord->ErrorInfo());
            }
        };
        extern TaskHub *g_TaskHub;
        g_TaskHub->Add(fnDBOp);
    };

    pSession->Read(sizeof(CMLogin), fnProcessLogin);
}

#include <cstdarg>

#include "log.hpp"
#include "message.hpp"
#include "database.hpp"
#include "mainwindow.hpp"
#include "monoserver.hpp"

#include "databaseconfigurewindow.hpp"

MonoServer::MonoServer()
    : m_SessionIO(nullptr)
{
}

MonoServer::~MonoServer()
{
}

void MonoServer::ExtendLogBuf(size_t nNewSize)
{
    if(nNewSize > m_LogBufSize){
        delete[] m_LogBuf;
        size_t nNewSize8 = ((nNewSize + 7) / 8) * 8;
        m_LogBuf     = new char[nNewSize8];
        m_LogBufSize = nNewSize8;
    }
}

void MonoServer::AddLog(int nLogType, const char *szFormat, ...)
{
    extern MainWindow *g_MainWindow;

    ExtendLogBuf(128);

    va_list ap;

    int nMaxParseCount = 0;

    // actually it only needs 2 rounds at most
    while(10 > nMaxParseCount++){

        va_start(ap, szFormat);

        int nRes = std::vsnprintf(m_LogBuf, m_LogBufSize, szFormat, ap);

        va_end(ap);

        if(nRes > -1 && (size_t)nRes < m_LogBufSize){
            // additional '\0' takes one char
            // everything works
            g_MainWindow->AddLog(nLogType, m_LogBuf);
            // extern Log *g_Log;
            // switch(nLogType){
            //     case 1:
            //         {
            //             g_Log->AddLog(LOGTYPE_WARNING, "%s", m_LogBuf);
            //             break;
            //         }
            //     default:
            //         {
            //             g_Log->AddLog(LOGTYPE_INFO, "%s", m_LogBuf);
            //             break;
            //         }
            }
            return;

        }else if(nRes < 0){
            // error occurs in parsing log
            break;

        }else{
            // we need a larger buffer
            ExtendLogBuf(nRes + 1);
        }
    }

    g_MainWindow->AddLog(3, "error in parsing log message");
}

void MonoServer::CreateDBConnection()
{
    extern DatabaseConfigureWindow *g_DatabaseConfigureWindow;

    // delete m_UserInfoDB;

    m_DBConnection = new DBConnection(
            g_DatabaseConfigureWindow->DatabaseIP(),
            g_DatabaseConfigureWindow->UserName(),
            g_DatabaseConfigureWindow->Password(),
            g_DatabaseConfigureWindow->DatabaseName(),
            g_DatabaseConfigureWindow->DatabasePort());

    if(m_DBConnection->Valid()){
        AddLog(0, "Connect to Database (%s:%d) successfully", 
                g_DatabaseConfigureWindow->DatabaseIP(),
                g_DatabaseConfigureWindow->DatabasePort());
    }else{
        AddLog(2, "Can't connect to Database (%s:%d)", 
                g_DatabaseConfigureWindow->DatabaseIP(),
                g_DatabaseConfigureWindow->DatabasePort());
    }
}

void MonoServer::Launch()
{
    CreateDBConnection();

    // all-set, start to accept connections from clients
    extern ServerConfigureWindow *g_ServerConfigureWindow;
    int nPort = g_ServerConfigureWindow->Port();

    auto fnOperateHC = [this](uint8_t nMsgHC, Session *pSession){
        OnReadHC(nMsgHC, pSession);
    };

    m_SessionIO = new SessionIO(nPort, fnOperateHC);
    m_SessionIO->Launch();
}

void MonoServer::OnReadHC(uint8_t nMsgHC, Session *pSession)
{
    switch(nMsgHC){
        case CM_PING : { OnPing (pSession); break; }
        case CM_LOGIN: { OnLogin(pSession); break; }
        default: break;
    }
}

// void MonoServer::OnServerRecv(const Message &stMessage, Session &stSession)
// {
//     extern MainWindow *g_MainWindow;
//     // g_MainWindow->AddLog(0, "receive message from scene server.");
//     switch(stMessage.Index()){
//         case SERVERMT_MAPNAME:
//             {
//                 std::lock_guard<std::mutex> stGuard(m_SceneServerMapMutex);
//                 m_SceneServerMap[stSession.ID()] = (char *)stMessage.Body();
//                 break;
//             }
//         case SERVERMT_BROADCAST:
//             {
//                 ServerMessageBroadcast stSMB;
//                 std::memcpy(&stSMB, stMessage.Body(), sizeof(stSMB));
//                 int nSID = stSMB.nSID;
//                 int nUID = stSMB.nUID;
//                 Message stPackedMSG;
//                 std::memcpy(stPackedMSG.Data(), stSMB.Data, stSMB.nLen);
//                 switch(stPackedMSG.Index()){
//                     case SERVERMT_ACTORBASEINFO:
//                         {
//                             ClientMessageActorBaseInfo stCMABI;
//                             ServerMessageActorBaseInfo stSMABI;
//                             std::memcpy(&stSMABI, stPackedMSG.Body(), sizeof(stSMABI));
//                             stCMABI.nSID       = stSMABI.nSID;
//                             stCMABI.nUID       = stSMABI.nUID;
//                             stCMABI.nGenTime   = stSMABI.nGenTime;
//                             stCMABI.nX         = stSMABI.nX;
//                             stCMABI.nY         = stSMABI.nY;
//                             stCMABI.nState     = stSMABI.nState;
//                             stCMABI.nDirection = stSMABI.nDirection;
//
//                             Message stMSG;
//                             stMSG.Set(CLIENTMT_ACTORBASEINFO, stCMABI);
//                             {
//                                 std::lock_guard<std::mutex> stGuard(m_UIDLocationMutex);
//                                 if(m_UIDLocation.find(nUID) != m_UIDLocation.end()){
//                                     m_ClientSessionAcceptor.Deliver(m_UIDLocation[nUID].first, stMSG);
//                                 }else{
//                                     // TODO error here
//                                 }
//                             }
//                         }
//                 }
//                 break;
//             }
//         case SERVERMT_ACTORBASEINFO:
//             {
//                 break;
//             }
//         default:
//             {
//                 extern MainWindow *g_MainWindow;
//                 std::string szInfo = "Unknown message ";
//                 szInfo += std::to_string(stMessage.Index());
//                 szInfo += " from scene server (";
//                 szInfo += stSession.IP();
//                 szInfo += ":";
//                 szInfo += std::to_string(stSession.Port());
//                 szInfo += ") dropped";
//                 g_MainWindow->AddLog(1, szInfo.c_str());
//                 break;
//             }
//     }
// }

// void MonoServer::OnClientRecv(const Message &stMessage, Session &stSession)
// {
//     extern MainWindow *g_MainWindow;
//     switch(stMessage.Index()){
//         case CLIENTMT_LOGIN:
//             {
//                 DoLogin(stMessage, stSession);
//                 break;
//             }
//         default:
//             {
//                 extern MainWindow *g_MainWindow;
//                 std::string szInfo = "Unknown message ";
//                 szInfo += std::to_string(stMessage.Index());
//                 szInfo += " from client (";
//                 szInfo += stSession.IP();
//                 szInfo += ":";
//                 szInfo += std::to_string(stSession.Port());
//                 szInfo += ") dropped";
//                 g_MainWindow->AddLog(1, szInfo.c_str());
//                 break;
//             }
//     }
// }

// bool MonoServer::DoLogin(const Message &stMessage, Session &stSession)
// {
//     std::string szInfo = "login requested from (";
//     szInfo += stSession.IP();
//     szInfo += ":";
//     szInfo += std::to_string(stSession.Port());
//     szInfo += ")";
//     extern MainWindow *g_MainWindow;
//     g_MainWindow->AddLog(0, szInfo.c_str());
//
//     ClientMessageLogin stTmpCM;
//     std::memcpy(&stTmpCM, stMessage.Body(), sizeof(stTmpCM));
//     std::string szID       = stTmpCM.szID;
//     std::string szPassword = stTmpCM.szPWD;
//
//     auto pRecord = m_UserInfoDB->CreateDBRecord();
//     char szQueryCmd[512];
//     std::sprintf(szQueryCmd,
//             "select * from userinfo where fld_id = '%s' and fld_pwd = '%s'",
//             szID.c_str(), szPassword.c_str());
//
//     if(pRecord->Execute(szQueryCmd)){
//         if(pRecord->RowCount() == 0){
//             Message stFailMessage;
//             stFailMessage.SetIndex(CLIENTMT_LOGINFAIL);
//             stSession.Deliver(stFailMessage);
//         }else{
//             Message stSucceedMessage;
//             ClientMessageLoginSucceed stTmpCM;
//
//             pRecord->Fetch();
//
//             std::strcpy(stTmpCM.szCharName, pRecord->Get("fld_name"));
//             std::strcpy(stTmpCM.szMapName, pRecord->Get("fld_map"));
//
//             stTmpCM.nUID       = std::atoi(pRecord->Get("fld_uid"));
//             stTmpCM.nSID       = std::atoi(pRecord->Get("fld_sid"));
//             // stTmpCM.nLevel     = std::atoi(pRecord->Get("fld_level"));
//             stTmpCM.nMapX      = std::atoi(pRecord->Get("fld_x"));
//             stTmpCM.nMapY      = std::atoi(pRecord->Get("fld_y"));
//             stTmpCM.nDirection = std::atoi(pRecord->Get("fld_direction"));
//
//             stSucceedMessage.Set(CLIENTMT_LOGINSUCCEED, stTmpCM);
//             // don't send it here since we need to check map is available or not
//             // stSession.Deliver(stSucceedMessage);
//
//             // then we need to also send message to scene server
//             {
//                 std::string szMapName = stTmpCM.szMapName;
//                 std::lock_guard<std::mutex> stGuard(m_SceneServerMapMutex);
//                 for(auto &p:m_SceneServerMap){
//                     if(p.second == szMapName){
//                         // add record to list
//                         {
//                             std::lock_guard<std::mutex> stGuard(m_UIDLocationMutex);
//                             // since we find the map, then we assume the list is available
//                             m_UIDLocation[stTmpCM.nUID] = {stSession.ID(), p.first};
//                         }
//                         // talk to client: you are ok
//                         stSession.Deliver(stSucceedMessage);
//                         // talk to scene server: new human
//                         ServerMessageAddHuman stSMAH;
//                         stSMAH.nUID = stTmpCM.nUID;
//                         Message stMSG;
//                         stMSG.Set(SERVERMT_ADDHUMAN, stSMAH);
//                         m_SceneServerSessionAcceptor.Deliver(p.first, stMSG);
//                         return true;
//                     }
//                 }
//             }
//             // no available map
//             Message stMSGMapError;
//             stMSGMapError.SetIndex(CLIENTMT_MAPERROR);
//             stSession.Deliver(stMSGMapError);
//             return false;
//         }
//     }else{
//         char szRetInfo[512];
//         std::sprintf(szRetInfo, "SQL ERROR: (%d: %s)", pRecord->ErrorID(), pRecord->ErrorInfo());
//         g_MainWindow->AddLog(2, szRetInfo);
//     }
//
//     return true;
// }

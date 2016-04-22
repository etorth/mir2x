/*
 * =====================================================================================
 *
 *       Filename: monoserver.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 04/22/2016 15:27:18
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
#include <cstdarg>

#include "log.hpp"
#include "taskhub.hpp"
#include "eventtaskhub.hpp"
#include "message.hpp"
#include "database.hpp"
#include "mainwindow.hpp"
#include "monoserver.hpp"

#include "databaseconfigurewindow.hpp"

MonoServer::MonoServer()
    : m_SessionHub(nullptr)
    , m_ObjectUID(1)
    , m_LogBufSize(0)
    , m_LogBuf(nullptr)
{
    m_StartTime = std::chrono::system_clock::now();
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
        AddLog(LOGTYPE_INFO, "Connect to Database (%s:%d) successfully", 
                g_DatabaseConfigureWindow->DatabaseIP(),
                g_DatabaseConfigureWindow->DatabasePort());
    }else{
        AddLog(LOGTYPE_WARNING, "Can't connect to Database (%s:%d)", 
                g_DatabaseConfigureWindow->DatabaseIP(),
                g_DatabaseConfigureWindow->DatabasePort());
    }
}

void MonoServer::Launch()
{
    // 1. create db connection
    delete m_DBConnection;
    CreateDBConnection();

    // 2. load monster info
    m_MonsterRaceInfoV.clear();
    InitMonsterRace();
    InitMonsterItem();

    // 3. start AI loop
   
    extern TaskHub *g_TaskHub;
    // g_TaskHub->Shutdown();
    // 3. all-set, start to accept connections from clients
    extern ServerConfigureWindow *g_ServerConfigureWindow;
    int nPort = g_ServerConfigureWindow->Port();

    auto fnOperateHC = [this](uint8_t nMsgHC, Session *pSession){
        OnReadHC(nMsgHC, pSession);
    };

    delete m_SessionHub;
    m_SessionHub = new SessionHub(nPort, fnOperateHC);
    m_SessionHub->Launch();

    extern TaskHub *g_TaskHub;
    g_TaskHub->Launch();

    extern EventTaskHub *g_EventTaskHub;
    g_EventTaskHub->Launch();

    // TODO
    // dead lock when there is too many monsters???
    AddMonster(1, 1);
    AddMonster(1, 1);
    AddMonster(1, 1);
    AddMonster(1, 1);
}

void MonoServer::OnReadHC(uint8_t nMsgHC, Session *pSession)
{
    switch(nMsgHC){
        case CM_PING : { OnPing (pSession); break; }
        case CM_LOGIN: { OnLogin(pSession); break; }
        default: break;
    }
}

uint32_t MonoServer::GetTickCount()
{
    // ... so..
    return (uint32_t)std::chrono::duration_cast<
        std::chrono::milliseconds>(std::chrono::system_clock::now() - m_StartTime).count();
}

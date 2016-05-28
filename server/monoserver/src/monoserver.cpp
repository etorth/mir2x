/*
 * =====================================================================================
 *
 *       Filename: monoserver.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 05/28/2016 01:36:49
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
#include <vector>
#include <cstdarg>
#include <cstdlib>

#include "log.hpp"
#include "dbpod.hpp"
#include "taskhub.hpp"
#include "message.hpp"
#include "monster.hpp"
#include "database.hpp"
#include "mainwindow.hpp"
#include "monoserver.hpp"
#include "eventtaskhub.hpp"

#include "threadpn.hpp"
#include "servicecore.hpp"
#include "databaseconfigurewindow.hpp"

MonoServer::MonoServer()
    : m_LogBuf(nullptr)
    , m_LogBufSize(0)
    , m_ObjectUID(1)
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

void MonoServer::AddLog(const std::array<std::string, 4> &stLogDesc, const char *szLogFormat, ...)
{
    std::lock_guard<std::mutex> stGuard(m_LogLock);

    extern Log *g_Log;
    extern MainWindow *g_MainWindow;

    va_list ap;
    int nMaxParseCount = 0;
    int nLogType = std::atoi(stLogDesc[0].c_str());

    ExtendLogBuf(128);

    // actually it only needs 2 rounds at most
    while(10 > nMaxParseCount++){

        va_start(ap, szLogFormat);
        int nRes = std::vsnprintf(m_LogBuf, m_LogBufSize, szLogFormat, ap);
        va_end(ap);

        if(nRes > -1 && (size_t)nRes < m_LogBufSize){
            // additional '\0' takes one char, everything works
            if(nLogType != Log::LOGTYPEV_DEBUG){
                g_MainWindow->AddLog(nLogType, m_LogBuf);
            }
            g_Log->AddLog(stLogDesc, m_LogBuf);
            return;
        }else if(nRes < 0){
            // error occurs in parsing log
            break;
        }else{
            // we need a larger buffer
            ExtendLogBuf(nRes + 1);
        }
    }

    const char *szLogError = "MonoServer::AddLog(): Error in parsing log message";
    g_MainWindow->AddLog(3, szLogError);
    g_Log->AddLog(stLogDesc, szLogError);
}

void MonoServer::CreateDBConnection()
{
    extern DBPodN *g_DBPodN;
    extern DatabaseConfigureWindow *g_DatabaseConfigureWindow;

    if(g_DBPodN->Launch(
            g_DatabaseConfigureWindow->DatabaseIP(),
            g_DatabaseConfigureWindow->UserName(),
            g_DatabaseConfigureWindow->Password(),
            g_DatabaseConfigureWindow->DatabaseName(),
            g_DatabaseConfigureWindow->DatabasePort())){
        AddLog(LOGTYPE_WARNING, "DBPod can't connect to Database (%s:%d)", 
                g_DatabaseConfigureWindow->DatabaseIP(),
                g_DatabaseConfigureWindow->DatabasePort());
        // no database we just restart the monoserver
        Restart();
    }else{
        AddLog(LOGTYPE_INFO, "Connect to Database (%s:%d) successfully", 
                g_DatabaseConfigureWindow->DatabaseIP(),
                g_DatabaseConfigureWindow->DatabasePort());
    }
}

void MonoServer::CreateServiceCore()
{
    delete m_ServiceCore;
    m_ServiceCore = new ServiceCore();
    m_SCAddress = m_ServiceCore->Activate();
}

void MonoServer::StartNetwork()
{
    extern NetPodN *g_NetPodN;
    extern ServerConfigureWindow *g_ServerConfigureWindow;

    uint32_t nPort = g_ServerConfigureWindow->Port();
    if(g_NetPodN->Launch(nPort, m_SCAddress)){
        AddLog(LOGTYPE_WARNING, "launching network failed");
        Restart();
    }
}

void MonoServer::Launch()
{
    // 1. create db connection
    CreateDBConnection();

    // 2. load monster info
    LoadMonsterRecord();

    // 3. create service core
    CreateServiceCore();

    // 4. start network
    StartNetwork();

    extern EventTaskHub *g_EventTaskHub;
    g_EventTaskHub->Launch();

    uint32_t nUID       = 0;
    uint32_t nAddTime   = 0;
    uint32_t nMonsterID = 0;

    {
        nUID       = GetUID();
        nAddTime   = GetTimeTick();
        nMonsterID = 1;

        AddMonster(1, nUID, nAddTime, 1, 765, 573, false);
        if(MonsterValid(nUID, nAddTime)){
            AddLog(LOGTYPE_INFO,
                    "Added: (MonsterID, UID, AddTime) = (%d, %d, %d)", nMonsterID, nUID, nAddTime);
        }
    }

    {
        nUID       = GetUID();
        nAddTime   = GetTimeTick();
        nMonsterID = 1;

        AddMonster(1, nUID, nAddTime, 1, 442, 713, false);
        if(MonsterValid(nUID, nAddTime)){
            AddLog(LOGTYPE_INFO,
                    "Added: (MonsterID, UID, AddTime) = (%d, %d, %d)", nMonsterID, nUID, nAddTime);
        }
    }

    {
        nUID       = GetUID();
        nAddTime   = GetTimeTick();
        nMonsterID = 1;

        AddMonster(1, nUID, nAddTime, 1, 836, 530, false);
        if(MonsterValid(nUID, nAddTime)){
            AddLog(LOGTYPE_INFO,
                    "Added: (MonsterID, UID, AddTime) = (%d, %d, %d)", nMonsterID, nUID, nAddTime);
        }
    }

    {
        nUID       = GetUID();
        nAddTime   = GetTimeTick();
        nMonsterID = 1;

        AddMonster(1, nUID, nAddTime, 1, 932, 622, false);
        if(MonsterValid(nUID, nAddTime)){
            AddLog(LOGTYPE_INFO,
                    "Added: (MonsterID, UID, AddTime) = (%d, %d, %d)", nMonsterID, nUID, nAddTime);
        }
    }

    // AddMonster(1, nUID, nAddTime, 1, 765, 573, false);
    // AddMonster(1, nUID, nAddTime, 1, 442, 713, false);
    // AddMonster(1, nUID, nAddTime, 1, 836, 530, false);
    // AddMonster(1, nUID, nAddTime, 1, 932, 622, false);

    // TODO
    // dead lock when there is too many monsters???
}

void MonoServer::Restart()
{
    exit(0);
}

// I have to put it here, since in actorpod.hpp I used MonoServer::AddLog()
// then in monoserver.hpp if I use monster.hpp which includes actorpod.hpp
// it won't compile
//
// and it's good for me to make monoserver.hpp to be compact by moving these
// constant variables out
static std::vector<MONSTERRACEINFO> s_MonsterRaceInfoV;

bool MonoServer::InitMonsterRace()
{
    extern DBPodN *g_DBPodN;
    auto pRecord = g_DBPodN->CreateDBHDR();

    if(!pRecord){
        AddLog(LOGTYPE_WARNING, "create database handler failed");
        return false;
    }

    if(!pRecord->Execute("select * from mir2x.tbl_monster order by fld_index")){
        AddLog(LOGTYPE_WARNING, "SQL ERROR: (%d: %s)", pRecord->ErrorID(), pRecord->ErrorInfo());
        return false;
    }

    if(pRecord->RowCount() < 1){
        AddLog(LOGTYPE_INFO, "no monster info found");
        return true;
    }

    AddLog(LOGTYPE_INFO, "starting add monster info:");

    while(pRecord->Fetch()){
        MONSTERRACEINFO stRaceInfo;
        // 1. record the monster race info
        stRaceInfo.Name        = std::string(pRecord->Get("fld_name"));
        stRaceInfo.Index       = std::atoi(pRecord->Get("fld_index"));
        stRaceInfo.Race        = std::atoi(pRecord->Get("fld_race"));
        stRaceInfo.LID         = std::atoi(pRecord->Get("fld_lid"));
        stRaceInfo.Undead      = std::atoi(pRecord->Get("fld_undead"));
        stRaceInfo.Level       = std::atoi(pRecord->Get("fld_level"));
        stRaceInfo.HP          = std::atoi(pRecord->Get("fld_hp"));
        stRaceInfo.MP          = std::atoi(pRecord->Get("fld_mp"));
        stRaceInfo.AC          = std::atoi(pRecord->Get("fld_ac"));
        stRaceInfo.MAC         = std::atoi(pRecord->Get("fld_mac"));
        stRaceInfo.DC          = std::atoi(pRecord->Get("fld_dc"));
        stRaceInfo.AttackSpead = std::atoi(pRecord->Get("fld_attackspeed"));
        stRaceInfo.WalkSpead   = std::atoi(pRecord->Get("fld_walkspeed"));
        stRaceInfo.Spead       = std::atoi(pRecord->Get("fld_speed"));
        stRaceInfo.Hit         = std::atoi(pRecord->Get("fld_hit"));
        stRaceInfo.ViewRange   = std::atoi(pRecord->Get("fld_viewrange"));
        stRaceInfo.RaceIndex   = std::atoi(pRecord->Get("fld_raceindex"));
        stRaceInfo.Exp         = std::atoi(pRecord->Get("fld_exp"));
        stRaceInfo.Escape      = std::atoi(pRecord->Get("fld_escape"));
        stRaceInfo.Water       = std::atoi(pRecord->Get("fld_water"));
        stRaceInfo.Fire        = std::atoi(pRecord->Get("fld_fire"));
        stRaceInfo.Wind        = std::atoi(pRecord->Get("fld_wind"));
        stRaceInfo.Light       = std::atoi(pRecord->Get("fld_light"));
        stRaceInfo.Earth       = std::atoi(pRecord->Get("fld_earth"));

        // 2. make a room in the global table
        s_MonsterRaceInfoV.resize(stRaceInfo.Index + 1, -1);

        // 3. store the result
        s_MonsterRaceInfoV[stRaceInfo.Index] = stRaceInfo;

        // 3. log it
        AddLog(LOGTYPE_INFO,
                "monster added, index = %d, name = %s.", stRaceInfo.Index, stRaceInfo.Name.c_str());
    }

    AddLog(LOGTYPE_INFO, "finished monster info: %d monster added", pRecord->RowCount());
    return true;
}

bool MonoServer::InitMonsterItem()
{
    extern DBPodN *g_DBPodN;
    auto pRecord = g_DBPodN->CreateDBHDR();
    if(!pRecord->Execute("select * from mir2x.tbl_monsteritem")){
        AddLog(LOGTYPE_WARNING, "SQL ERROR: (%d: %s)", pRecord->ErrorID(), pRecord->ErrorInfo());
        return false;
    }

    if(pRecord->RowCount() < 1){
        AddLog(LOGTYPE_INFO, "no monster item found");
        return true;
    }

    AddLog(LOGTYPE_INFO, "starting add monster item:");

    while(pRecord->Fetch()){
        MONSTERITEMINFO stItemInfo;
        // 1. get the item desc
        stItemInfo.MonsterIndex = std::atoi(pRecord->Get("fld_monster"));
        stItemInfo.Type         = std::atoi(pRecord->Get("fld_type"));
        stItemInfo.Chance       = std::atoi(pRecord->Get("fld_chance"));
        stItemInfo.Count        = std::atoi(pRecord->Get("fld_count"));
        // 2. make a room for it
        if(true 
                && stItemInfo.MonsterIndex > 0
                && stItemInfo.MonsterIndex < (int)s_MonsterRaceInfoV.size()){
            s_MonsterRaceInfoV[stItemInfo.MonsterIndex].ItemV.push_back(stItemInfo);
        }
    }
    AddLog(LOGTYPE_INFO, "finished monster item: %d added", pRecord->RowCount());
    return true;
}

// request adding a new monster without response, after this the externa layer
// can check whether the monster exists
//
// TODO & TBD
//
// I don't like the response here, since this introduce huge complexity
// if I am waiting for a response, then following thing should be considered
//
// 1. map is not ready
// 2. requested RM on the map is not ready
//
// to handle these we have use many response callback, and since when RM is
// not ready it's in QUERY_PENDING state, I have to use anomyous trigger to
// handle it.
//
// instead I specified the monster with (UID, AddTime), and check whether
// this monster exist? this should be much simpler
void MonoServer::AddMonster(uint32_t nMonsterID,
        uint32_t nUID, uint32_t nAddTime, uint32_t nMapID, int nX, int nY, bool bAllowVoid)
{
    AMAddCharObject stAMACO;
    stAMACO.Type = OBJECT_MONSTER;

    stAMACO.Common.UID       = nUID;
    stAMACO.Common.AddTime   = nAddTime;
    stAMACO.Common.MapID     = nMapID;
    stAMACO.Common.MapX      = nX;
    stAMACO.Common.MapY      = nY;
    stAMACO.Common.R         = 10; // TODO
    stAMACO.Common.AllowVoid = bAllowVoid;

    stAMACO.Monster.MonsterID = nMonsterID;

    SyncDriver().Forward({MPK_ADDCHAROBJECT, stAMACO}, m_SCAddress);
}

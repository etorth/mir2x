/*
 * =====================================================================================
 *
 *       Filename: monoserver.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 09/23/2017 22:57:45
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
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <cinttypes>
#include <FL/fl_ask.H>

#include "log.hpp"
#include "dbpod.hpp"
#include "taskhub.hpp"
#include "message.hpp"
#include "monster.hpp"
#include "database.hpp"
#include "threadpn.hpp"
#include "mapbindbn.hpp"
#include "uidrecord.hpp"
#include "mainwindow.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"
#include "eventtaskhub.hpp"
#include "commandwindow.hpp"
#include "databaseconfigurewindow.hpp"

MonoServer::MonoServer()
    : m_LogLock()
    , m_LogBuf()
    , m_ServiceCore(nullptr)
    , m_GlobalUID {1}
    , m_UIDArray()
    , m_StartTime(std::chrono::system_clock::now())
{}

void MonoServer::AddLog(const std::array<std::string, 4> &stLogDesc, const char *szLogFormat, ...)
{
    int nLogSize = 0;
    int nLogType = std::atoi(stLogDesc[0].c_str());

    auto fnRecordLog = [this, &stLogDesc](int nLogType, const char *szLogInfo)
    {
        extern Log *g_Log;
        switch(nLogType){
            case Log::LOGTYPEV_DEBUG:
                {
                    g_Log->AddLog(stLogDesc, szLogInfo);
                    break;
                }
            default:
                {
                    g_Log->AddLog(stLogDesc, szLogInfo);
                    {
                        std::lock_guard<std::mutex> stLockGuard(m_LogLock);
                        m_LogBuf.push_back((char)(nLogType));
                        m_LogBuf.insert(m_LogBuf.end(), szLogInfo, szLogInfo + std::strlen(szLogInfo) + 1);
                    }
                    NotifyGUI("FlushBrowser");
                    break;
                }
        }
    };

    // 1. try static buffer
    //    give an enough size so we can hopefully stop here
    {
        char szSBuf[1024];

        va_list ap;
        va_start(ap, szLogFormat);
        nLogSize = std::vsnprintf(szSBuf, (sizeof(szSBuf) / sizeof(szSBuf[0])), szLogFormat, ap);
        va_end(ap);

        if(nLogSize >= 0){
            if((size_t)(nLogSize + 1) < (sizeof(szSBuf) / sizeof(szSBuf[0]))){
                fnRecordLog(nLogType, szSBuf);
                return;
            }else{
                // do nothing
                // have to try the dynamic buffer method
            }
        }else{
            fnRecordLog(Log::LOGTYPEV_FATAL, (std::string("Parse log info failed: ") + szLogFormat).c_str());
            return;
        }
    }

    // 2. try dynamic buffer
    //    use the parsed buffer size above to get enough memory
    while(true){
        std::vector<char> szDBuf(nLogSize + 1 + 64);

        va_list ap;
        va_start(ap, szLogFormat);
        nLogSize = std::vsnprintf(&(szDBuf[0]), szDBuf.size(), szLogFormat, ap);
        va_end(ap);

        if(nLogSize >= 0){
            if((size_t)(nLogSize + 1) < szDBuf.size()){
                fnRecordLog(nLogType, &(szDBuf[0]));
                return;
            }else{
                szDBuf.resize(nLogSize + 1 + 64);
            }
        }else{
            fnRecordLog(Log::LOGTYPEV_FATAL, (std::string("Parse log info failed: ") + szLogFormat).c_str());
            return;
        }
    }
}

void MonoServer::AddCWLog(uint32_t nCWID, int nLogType, const char *szPrompt, const char *szLogFormat, ...)
{
    auto fnCWRecordLog = [this](uint32_t nCWID, int nLogType, const char *szPrompt, const char *szLogMsg){
        if(true
                && (nCWID)
                && (nLogType == 0 || nLogType == 1 || nLogType == 2)){

            szPrompt = szPrompt ? szPrompt : "";
            szLogMsg = szLogMsg ? szLogMsg : "";

            // we should lock the internal buffer record
            // we won't assess any gui instance in this function
            {
                std::lock_guard<std::mutex> stLockGuard(m_CWLogLock);
                m_CWLogBuf.insert(m_CWLogBuf.end(), (char *)(&nCWID), (char *)(&nCWID) + sizeof(nCWID));
                m_CWLogBuf.push_back((char)(nLogType));

                m_CWLogBuf.insert(m_CWLogBuf.end(), szPrompt, szPrompt + std::strlen(szPrompt) + 1);
                m_CWLogBuf.insert(m_CWLogBuf.end(), szLogMsg, szLogMsg + std::strlen(szLogMsg) + 1);
            }
            NotifyGUI("FlushCWBrowser");
        }
    };

    int nLogSize = 0;

    // 1. try static buffer
    //    give a enough size so we can hopefully stop here
    {
        char szSBuf[1024];

        va_list ap;
        va_start(ap, szLogFormat);
        nLogSize = std::vsnprintf(szSBuf, (sizeof(szSBuf) / sizeof(szSBuf[0])), szLogFormat, ap);
        va_end(ap);

        if(nLogSize >= 0){
            if((size_t)(nLogSize + 1) < (sizeof(szSBuf) / sizeof(szSBuf[0]))){
                fnCWRecordLog(nCWID, nLogType, szPrompt, szSBuf);
                return;
            }else{
                // do nothing
                // have to try the dynamic buffer method
            }
        }else{
            AddLog(LOGTYPE_WARNING, "Parse command window log info failed: %s", szLogFormat);
            return;
        }
    }

    // 2. try dynamic buffer
    //    use the parsed buffer size above to get enough memory
    while(true){
        std::vector<char> szDBuf(nLogSize + 1 + 64);

        va_list ap;
        va_start(ap, szLogFormat);
        nLogSize = std::vsnprintf(&(szDBuf[0]), szDBuf.size(), szLogFormat, ap);
        va_end(ap);

        if(nLogSize >= 0){
            if((size_t)(nLogSize + 1) < szDBuf.size()){
                fnCWRecordLog(nCWID, nLogType, szPrompt, &(szDBuf[0]));
                return;
            }else{
                szDBuf.resize(nLogSize + 1 + 64);
            }
        }else{
            AddLog(LOGTYPE_WARNING, "Parse command window log info failed: %s", szLogFormat);
            return;
        }
    }
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

void MonoServer::RegisterAMFallbackHandler()
{
    static struct FrameworkFallbackHandler
    {
        void Handler(const void *pData, const Theron::uint32_t, const Theron::Address stFromAddress)
        {
            // dangerous part
            // try to recover basic information of the message
            // don't refer to the data field here, it could be dynamically allcoated
            MessagePack stRawMPK;
            std::memcpy(&stRawMPK, pData, sizeof(stRawMPK));

            AMBadActorPod stAMBAP;
            stAMBAP.Type    = stRawMPK.Type();
            stAMBAP.ID      = stRawMPK.ID();
            stAMBAP.Respond = stRawMPK.Respond();

            // we know which actor sent this message
            // but we lost the information that which actor it sent to
            SyncDriver().Forward({MPK_BADACTORPOD, stAMBAP}, stFromAddress, stRawMPK.ID());
        }
    }stFallbackHandler;

    extern Theron::Framework *g_Framework;
    g_Framework->SetFallbackHandler(&stFallbackHandler, &FrameworkFallbackHandler::Handler);
}

void MonoServer::LoadMapBinDBN()
{
    extern ServerConfigureWindow *g_ServerConfigureWindow;
    std::string szMapPath = g_ServerConfigureWindow->GetMapPath();

    extern MapBinDBN *g_MapBinDBN;
    if(!g_MapBinDBN->Load(szMapPath.c_str())){
        AddLog(LOGTYPE_FATAL, "Failed to load mapbindbn");
    }
}

void MonoServer::CreateServiceCore()
{
    delete m_ServiceCore;
    m_ServiceCore = new ServiceCore();
    m_ServiceCore->Activate();
}

void MonoServer::StartNetwork()
{
    extern NetDriver *g_NetDriver;
    extern ServerConfigureWindow *g_ServerConfigureWindow;

    uint32_t nPort = g_ServerConfigureWindow->Port();
    if(g_NetDriver->Launch(nPort, m_ServiceCore->GetAddress())){
        AddLog(LOGTYPE_FATAL, "Failed to launch the network");
        Restart();
    }
}

void MonoServer::Launch()
{
    CreateDBConnection();
    LoadMonsterRecord();
    RegisterAMFallbackHandler();

    LoadMapBinDBN();

    CreateServiceCore();
    StartNetwork();

    extern EventTaskHub *g_EventTaskHub;
    g_EventTaskHub->Launch();
}

void MonoServer::Restart()
{
    // TODO: FLTK multi-threading support is weak, see:
    // http://www.fltk.org/doc-1.3/advanced.html#advanced_multithreading

    // Fl::awake() will send message to main loop
    // define the main loop to call exit(0) when pass 1 to main thread

    // this function itself can be called from
    //   1. main loop
    //   2. child thread
    NotifyGUI("Restart");
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
        AddLog(LOGTYPE_INFO, "monster added, index = %d, name = %s.", stRaceInfo.Index, stRaceInfo.Name.c_str());
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

bool MonoServer::AddMonster(uint32_t nMonsterID, uint32_t nMapID, int nX, int nY, bool bRandom)
{
    AMAddCharObject stAMACO;
    stAMACO.Type = TYPE_MONSTER;

    stAMACO.Common.MapID  = nMapID;
    stAMACO.Common.X      = nX;
    stAMACO.Common.Y      = nY;
    stAMACO.Common.Random = bRandom;

    stAMACO.Monster.MonsterID = nMonsterID;
    stAMACO.Monster.MasterUID = 0;
    AddLog(LOGTYPE_INFO, "Try to add monster, MonsterID = %d", nMonsterID);

    MessagePack stRMPK;
    SyncDriver().Forward({MPK_ADDCHAROBJECT, stAMACO}, m_ServiceCore->GetAddress(), &stRMPK);
    switch(stRMPK.Type()){
        case MPK_OK:
            {
                AddLog(LOGTYPE_INFO, "Add monster succeeds");
                return true;
            }
        case MPK_ERROR:
            {
                AddLog(LOGTYPE_WARNING, "Add monster failed");
                return false;
            }
        default:
            {
                AddLog(LOGTYPE_WARNING, "Unsupported message: %s", stRMPK.Name());
                return false;
            }
    }
}

std::vector<int> MonoServer::GetMapList()
{
    MessagePack stRMPK;
    SyncDriver().Forward(MPK_QUERYMAPLIST, m_ServiceCore->GetAddress(), &stRMPK);
    switch(stRMPK.Type()){
        case MPK_MAPLIST:
            {
                AMMapList stAMML;
                std::memcpy(&stAMML, stRMPK.Data(), sizeof(stAMML));

                std::vector<int> stMapList;
                for(size_t nIndex = 0; nIndex < sizeof(stAMML.MapList) / sizeof(stAMML.MapList[0]); ++nIndex){
                    if(stAMML.MapList[nIndex]){
                        stMapList.push_back((int)(stAMML.MapList[nIndex]));
                    }else{
                        break;
                    }
                }
                return stMapList;
            }
        default:
            {
                return {};
            }
    }
}

sol::optional<int> MonoServer::GetMonsterCount(int nMonsterID, int nMapID)
{
    // I have two ways to implement this function
    //  1. GetMapList() / GetMapUID() / GetMapAddress() / QueryMonsterCount()
    //  2. send QueryMonsterCount to ServiceCore, ServiceCore queries all maps and return the sum
    // currently using method-2

    if(true
            && nMapID     >= 0      // 0 means check all
            && nMonsterID >= 0){    // 0 means check all types

        // OK we send request to service core
        // and poll the result here till we get the sum

        AMQueryCOCount stAMQCOC;
        std::memset(&stAMQCOC, 0, sizeof(stAMQCOC));

        stAMQCOC.MapID                = (uint32_t)(nMapID);
        stAMQCOC.Check.Monster        = true;
        stAMQCOC.CheckParam.MonsterID = (uint32_t)(nMonsterID);

        MessagePack stRMPK;
        SyncDriver().Forward({MPK_QUERYCOCOUNT, stAMQCOC}, m_ServiceCore->GetAddress(), &stRMPK);
        switch(stRMPK.Type()){
            case MPK_COCOUNT:
                {
                    AMCOCount stAMCOC;
                    std::memcpy(&stAMCOC, stRMPK.Data(), sizeof(stAMCOC));

                    // may overflow
                    // should put some check here
                    return sol::optional<int>((int)(stAMCOC.Count));
                }
            case MPK_ERROR:
            default:
                {
                    break;
                }
        }
    }
    return sol::optional<int>();
}

void MonoServer::NotifyGUI(std::string szNotification)
{
    if(szNotification != ""){
        {
            std::lock_guard<std::mutex> stLockGuard(m_NotifyGUILock);
            m_NotifyGUIQ.push(szNotification);
        }
        Fl::awake((void *)(uintptr_t)(1));
    }
}

void MonoServer::ParseNotifyGUIQ()
{
    static auto fnGetTokenList = [](const std::string &szCommand) -> std::deque<std::string>
    {
        std::deque<std::string> stTokenList;

        size_t nCurrLoc = 0;
        while(nCurrLoc <= szCommand.size()){
            auto nLoc0 = szCommand.find_first_not_of(" \t", nCurrLoc);
            auto nLoc1 = szCommand.find_first_of    (" \t", nCurrLoc);

            if(nLoc0 == std::string::npos){
                // done parsing
                // there is no more tokens in the string
                break;
            }else if(nLoc1 == std::string::npos){           // last one
                // nLoc0 is OK
                // nLoc1 reaches the end
                // this is the last token in the list
                stTokenList.push_back(szCommand.substr(nLoc0));
                break;
            }else if(nLoc0 < nLoc1){
                // match cases
                // make sure nLoc0 < nLoc1 to avoid cases like: "    OK 1"
                stTokenList.push_back(szCommand.substr(nLoc0, nLoc1 - nLoc0));
                nCurrLoc = nLoc1;
                continue;
            }else{
                // case for nLoc0 > nLoc1, no equal here
                // cases like "   OK 1", take no token and move forward
                nCurrLoc = nLoc0;
                continue;
            }
        }
        return stTokenList;
    };


    while(true){
        std::string szCurrNotify;
        {
            std::lock_guard<std::mutex> stLockGuard(m_NotifyGUILock);
            if(m_NotifyGUIQ.empty()){ break; }

            szCurrNotify = m_NotifyGUIQ.front();
            m_NotifyGUIQ.pop();
        }

        auto stTokenList = fnGetTokenList(szCurrNotify);
        if(stTokenList.empty()){ continue; }

        // get a valid token list
        // parse the first token as command name

        if(false
                || stTokenList.front() == "exit"
                || stTokenList.front() == "Exit"
                || stTokenList.front() == "EXIT"){
            std::exit(0);
            return;
        }

        if(false
                || stTokenList.front() == "restart"
                || stTokenList.front() == "Restart"
                || stTokenList.front() == "RESTART"){
            fl_alert("%s", "System request for restart");
            std::exit(0);
            return;
        }
        
        if(false
                || stTokenList.front() == "flushbrowser"
                || stTokenList.front() == "FlushBrowser"
                || stTokenList.front() == "FLUSHBROWSER"){
            extern MonoServer *g_MonoServer;
            g_MonoServer->FlushBrowser();
            continue;
        }
        
        if(false
                || stTokenList.front() == "flushcwbrowser"
                || stTokenList.front() == "FlushCWBrowser"
                || stTokenList.front() == "FLUSHCWBROWSER"){
            extern MonoServer *g_MonoServer;
            g_MonoServer->FlushCWBrowser();
            continue;
        }

        if(false
                || stTokenList.front() == "exitcw"
                || stTokenList.front() == "ExitCW"
                || stTokenList.front() == "EXITCW"){
            stTokenList.pop_front();
            int nCWID = 0;
            try{
                nCWID = std::stoi(stTokenList.front());
            }catch(...){
                nCWID = 0;
            }

            if(nCWID > 0){
                extern MainWindow *g_MainWindow;
                g_MainWindow->DeleteCommandWindow(nCWID);
            }
            continue;
        }

        // unsupported command here
        // print into the log file and not exit here
        {
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Unsupported NotifyGUI command: %s", stTokenList.front().c_str());
            continue;
        }
    }
}

void MonoServer::FlushBrowser()
{
    std::lock_guard<std::mutex> stLockGuard(m_LogLock);
    {
        auto nCurrLoc = (size_t)(0);
        while(nCurrLoc < m_LogBuf.size()){
            extern MainWindow *g_MainWindow;
            g_MainWindow->AddLog((int)(m_LogBuf[nCurrLoc]), &(m_LogBuf[nCurrLoc + 1]));
            nCurrLoc += (1 + 1 + std::strlen(&(m_LogBuf[nCurrLoc + 1])));
        }
        m_LogBuf.clear();
    }
}

void MonoServer::FlushCWBrowser()
{
    std::lock_guard<std::mutex> stLockGuard(m_CWLogLock);
    {
        // internal structure of CWLogBuf:
        // [xxxx][x][..0][....0][xxxx][x][..0][....0] ....
        //
        // [xxxx]  : CWID
        // [x]     : LogType: 0 ~ 2
        // [..0]   : szPrompt
        // [....0] : szLogInfo

        size_t nCurrLoc = 0;
        while(nCurrLoc < m_CWLogBuf.size()){
            uint32_t nCWID = 0;
            std::memcpy(&nCWID, &(m_CWLogBuf[nCurrLoc]), sizeof(nCWID));

            auto nInfoLen0 = std::strlen(&(m_CWLogBuf[nCurrLoc + sizeof(nCWID) + 1]));
            auto nInfoLen1 = std::strlen(&(m_CWLogBuf[nCurrLoc + sizeof(nCWID) + 1 + nInfoLen0 + 1]));

            auto pInfo0 = &(m_CWLogBuf[nCurrLoc + sizeof(nCWID) + 1]);
            auto pInfo1 = &(m_CWLogBuf[nCurrLoc + sizeof(nCWID) + 1 + nInfoLen0 + 1]);

            extern MainWindow *g_MainWindow;
            g_MainWindow->AddCWLog(nCWID, (int)(m_CWLogBuf[nCurrLoc + sizeof(nCWID)]), pInfo0, pInfo1);
            nCurrLoc += (sizeof(nCWID) + 1 + nInfoLen0 + 1 + nInfoLen1 + 1);
        }
        m_CWLogBuf.clear();
    }
}

uint32_t MonoServer::GetUID()
{
    return m_GlobalUID.fetch_add(1);
}

bool MonoServer::LinkUID(uint32_t nUID, ServerObject *pObject)
{
    if(nUID && pObject){
        auto &rstRecord = m_UIDArray[nUID % m_UIDArray.size()];
        {
            std::lock_guard<std::mutex> stLockGuard(rstRecord.Lock);
            auto stFind = rstRecord.Record.find(nUID);
            if(stFind == rstRecord.Record.end()){
                rstRecord.Record[nUID] = pObject;
                return true;
            }else{
                AddLog(LOGTYPE_WARNING, "UIDArray duplicated UID: (%" PRIu32 ", %p, %p)", nUID, stFind->second, pObject);
                return false;
            }
        }
    }

    AddLog(LOGTYPE_WARNING, "Invalid argument LinkUID(UID = %" PRIu32 ", ServerObject = %p)", nUID, pObject);
    return false;
}

void MonoServer::EraseUID(uint32_t nUID)
{
    if(nUID){
        auto &rstRecord = m_UIDArray[nUID % m_UIDArray.size()];
        {
            std::lock_guard<std::mutex> stLockGuard(rstRecord.Lock);
            auto stFind = rstRecord.Record.find(nUID);
            if(stFind != rstRecord.Record.end()){
                if(stFind->second){
                    if(stFind->second->UID() != nUID){
                        AddLog(LOGTYPE_WARNING, "UIDArray mismatch: UID = (%" PRIu32 ", %" PRIu32 ")", nUID, stFind->second->UID());
                    }
                    delete stFind->second;
                }
                rstRecord.Record.erase(stFind);
            }
        }
    }
}

UIDRecord MonoServer::GetUIDRecord(uint32_t nUID)
{
    if(nUID){
        auto &rstUIDArrayEntry = m_UIDArray[nUID % m_UIDArray.size()];
        {
            std::lock_guard<std::mutex> stLockGuard(rstUIDArrayEntry.Lock);
            auto pRecord = rstUIDArrayEntry.Record.find(nUID);
            if(pRecord != rstUIDArrayEntry.Record.end()){
                if(pRecord->second){
                    if(pRecord->second->UID() == nUID){
                        // two solutions
                        // 1. for server object, define a virtual UIDRecord GetUIDRecord()
                        //    then here directly forward the result
                        //
                        // 2. maintain UID() for server object
                        //    maintain UID() and GetAddress() for classes from ActiveObject
                        //    then construct a temporary UIDRecord and return
                        //
                        // solution-1 : simpler but it introduces concept of address to server object
                        // solution-2 : means I should make both UID() and GetAddress() atomically accessable
                        //
                        // UID()        : OK by default
                        // GetAddress() : which calls m_ActorPod->GetAddress()
                        //                m_ActorPod could change when other threads accessing it
                        //
                        // solution:
                        // 1. we constrains that we can only access an UID if the object actively given it
                        //    means if we try to access object through GetUIDRecord(nUID), then the nUID is reported
                        //    by the object itself. rather than we do randomly draw an UID and access it
                        //
                        //    an UID can be deleted then we get an invalid UIDRecord
                        //    this behaves like malloc() / free(), any pointer try to free() should be from malloc()
                        //
                        //    this means if we trying to access ActiveObject::GetAddress(), its m_ActorPod has
                        //    already be initialized otherwise we can't get its UID
                        //
                        // 2. before deletion of active object we should call Deactivate() which calls m_ActorPod->Detach()
                        //    this helps to detach *this* from the actor thread of m_ActorPod, then deletion in other thread is OK
                        //
                        //    then deletion of m_ActorPod will wait if m_ActorPod is scheduled in actor threads
                        //
                        auto bActive = pRecord->second->ClassFrom<ActiveObject>();
                        auto stAddress = bActive ? ((ActiveObject *)(pRecord->second))->GetAddress() : Theron::Address::Null();
                        return {nUID, pRecord->second->GetInvarData(), stAddress, pRecord->second->ClassEntry()};
                    }else{
                        AddLog(LOGTYPE_WARNING, "UIDArray mismatch: UID = (%" PRIu32 ", %" PRIu32 ")", nUID, pRecord->second->UID());
                    }
                }
            }
        }
    }

    // for all other cases, return empty record
    // 1. provided uid as zero
    // 2. record doesn't exist
    // 3. record contains an empty pointer
    // 4. record mismatch
    static const std::vector<ServerObject::ClassCodeName> stNullEntry {};
    return UIDRecord(0, {}, Theron::Address::Null(), stNullEntry);
}

bool MonoServer::RegisterLuaExport(ServerLuaModule *pModule, uint32_t nCWID)
{
    if(true
            && pModule      // module to execute lua script
            && nCWID){      // command window id to echo all execution information

        // initialization before registration
        pModule->script(R"()");

        // register command exitServer
        // exit current server and free all related resource
        pModule->set_function("exitServer", [](){ exit(0); });

        // register command exit
        // exit current command window and free all related resource
        pModule->set_function("exit", [this, nCWID](){
            // 1. show exiting messages
            AddCWLog(nCWID, 0, "> ", "Command window is requested to exit now...");

            // 2. flush command windows
            //    we need to flush message before exit the command window
            //    otherwise next created command window may get them if it uses the same CWID
            NotifyGUI("FlushCWBrowser");
            NotifyGUI(std::string("ExitCW ") + std::to_string(nCWID));
        });

        // register command sleep
        // sleep current lua thread and return after the specified ms
        // can use posix.sleep(ms), but here use std::this_thread::sleep_for(x)
        pModule->set_function("sleep", [this, nCWID](int nSleepMS){
            if(nSleepMS > 0){
                std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMS));
            }
        });

        // register command printLine
        // print one line in command window
        // won't add message to log system, use addLog instead
        pModule->set_function("printLine", [this, nCWID](sol::object stLogType, sol::object stPrompt, sol::object stLogInfo){
            // use sol::object to accept arguments
            // otherwise for follow code it throws exception for type unmatch
            //      lua["f"] = [](int a){ return a; };
            //      lua.script("print f(\"hello world\")")
            // program crashes with exception.what() : expecting int, string provided
            if(true
                    && stLogType.is<int>()
                    && stPrompt.is<std::string>()
                    && stLogInfo.is<std::string>()){
                AddCWLog(nCWID, stLogType.as<int>(), stPrompt.as<std::string>().c_str(), stLogInfo.as<std::string>().c_str());
                return;
            }

            // invalid argument provided
            AddCWLog(nCWID, 2, ">>> ", "printLine(LogType: int, Prompt: string, LogInfo: string)");
        });

        // register command addLog
        // add to system log file and main window history window
        pModule->set_function("addLog", [this, nCWID](sol::object stLogType, sol::object stLogInfo){
            if(true
                    && stLogType.is<int>()
                    && stLogInfo.is<std::string>()){
                switch(stLogType.as<int>()){
                    case 0  : AddLog(LOGTYPE_INFO   , "%s", stLogInfo.as<std::string>().c_str()); return;
                    case 1  : AddLog(LOGTYPE_WARNING, "%s", stLogInfo.as<std::string>().c_str()); return;
                    default : AddLog(LOGTYPE_FATAL  , "%s", stLogInfo.as<std::string>().c_str()); return;
                }
            }

            // invalid argument provided
            AddCWLog(nCWID, 2, ">>> ", "addLog(LogType: int, LogInfo: string)");
        });

        // register command mapList
        // get a list for all active maps
        // return a table (userData) to lua for ipairs() check
        pModule->set_function("mapList", [this](sol::this_state stThisLua){
            return sol::make_object(sol::state_view(stThisLua), GetMapList());
        });

        // register command countMonster(monsterID, mapID)
        pModule->set_function("countMonster", [this, nCWID](int nMonsterID, int nMapID) -> int {
            auto nRet = GetMonsterCount(nMonsterID, nMapID).value_or(-1);
            if(nRet < 0){
                AddCWLog(nCWID, 2, ">>> ", "countMonster(MonsterID: int, MapID: int) failed");
            }
            return nRet;
        });

        // register command addMonster
        // will support add monster by monster name and map name
        // here we need to register a function to do the monster creation
        //      1. fnAddMonster(sol::variadic_args stVariadicArgs)
        //      2. fnAddMonster(int nMonsterID, int nMapID, sol::variadic_args stVariadicArgs)
        // I want to use method-1 in future, method-2 is more readable but can't handle the parameter issue:
        //      for i in mapList()
        //      do
        //          addMonster(i)
        //      end
        // here we get an exception from lua caught by sol2: ``std::bad_alloc"
        // but we want more detailed information like print the function usage out
        pModule->set_function("addMonster", [this, nCWID](int nMonsterID, int nMapID, sol::variadic_args stVariadicArgs) -> bool {
            auto fnPrintUsage = [this, nCWID]()
            {
                AddCWLog(nCWID, 2, ">>> ", "addMonster(MonsterID: int, MapID: int)");
                AddCWLog(nCWID, 2, ">>> ", "addMonster(MonsterID: int, MapID: int, X: int, Y: int)");
                AddCWLog(nCWID, 2, ">>> ", "addMonster(MonsterID: int, MapID: int, X: int, Y: int, Random: bool)");
            };

            std::vector<sol::object> stArgList(stVariadicArgs.begin(), stVariadicArgs.end());
            switch(stArgList.size()){
                case 0:
                    {
                        return AddMonster((uint32_t)(nMonsterID), (uint32_t)(nMapID), -1, -1, true);
                    }
                case 1:
                    {
                        fnPrintUsage();
                        return false;
                    }
                case 2:
                    {
                        if(true
                                && stArgList[0].is<int>()
                                && stArgList[1].is<int>()){
                            return AddMonster((uint32_t)(nMonsterID), (uint32_t)(nMapID), stArgList[0].as<int>(), stArgList[1].as<int>(), false);
                        }else{
                            fnPrintUsage();
                            return false;
                        }
                    }
                case 3:
                    {
                        if(true
                                && stArgList[0].is<int >()
                                && stArgList[1].is<int >()
                                && stArgList[2].is<bool>()){
                            return AddMonster((uint32_t)(nMonsterID), (uint32_t)(nMapID), stArgList[0].as<int>(), stArgList[1].as<int>(), stArgList[2].as<bool>());
                        }else{
                            fnPrintUsage();
                            return false;
                        }
                    }
                default:
                    {
                        fnPrintUsage();
                        return false;
                    }
            }
        });

        // register command ``listAllMap"
        // this command call mapList to get a table and print to CommandWindow
        pModule->script(R"#(
            function listAllMap ()
                for k, v in ipairs(mapList())
                do
                    printLine(0, "> ", tostring(v))
                end
            end
        )#");

        // register command ``help"
        // part-1: divide into two parts, part-1 create the table
        pModule->script(R"#(
            helpInfoTable = {
                mapList    = "return a list of all currently active maps",
                listAllMap = "print all map indices to current window"
            }
        )#");

        // part-2: make up the function to print the table entry
        pModule->script(R"#(
            function help (queryKey)
                if helpInfoTable[queryKey]
                then
                    printLine(0, "> ", helpInfoTable[queryKey])
                else
                    printLine(2, "> ", "No entry find for input")
                end
            end
        )#");
    }
    return false;
}

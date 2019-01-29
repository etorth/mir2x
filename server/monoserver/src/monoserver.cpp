/*
 * =====================================================================================
 *
 *       Filename: monoserver.cpp
 *        Created: 08/31/2015 10:45:48 PM
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
#include "actorpool.hpp"
#include "mapbindbn.hpp"
#include "syncdriver.hpp"
#include "mainwindow.hpp"
#include "monoserver.hpp"
#include "dispatcher.hpp"
#include "servicecore.hpp"
#include "eventtaskhub.hpp"
#include "commandwindow.hpp"
#include "databaseconfigurewindow.hpp"

extern Log *g_Log;
extern DBPodN *g_DBPodN;
extern ActorPool *g_ActorPool;
extern MapBinDBN *g_MapBinDBN;
extern NetDriver *g_NetDriver;
extern MonoServer *g_MonoServer;
extern MainWindow *g_MainWindow;
extern ServerConfigureWindow *g_ServerConfigureWindow;
extern DatabaseConfigureWindow *g_DatabaseConfigureWindow;

MonoServer::MonoServer()
    : m_LogLock()
    , m_LogBuf()
    , m_ServiceCore(nullptr)
    , m_CurrException()
    , m_HRTimer()
{}

void MonoServer::AddLog(const std::array<std::string, 4> &stLogDesc, const char *szLogFormat, ...)
{
    std::string szLog;
    bool bError = false;
    {
        va_list ap;
        va_start(ap, szLogFormat);

        try{
            szLog = str_vprintf(szLogFormat, ap);
        }catch(const std::exception &e){
            bError = true;
            szLog = str_printf("Exception caught in MonoServer::AddLog(\"%s\", ...): %s", szLogFormat, e.what());
        }

        va_end(ap);
    }

    int nLogType = bError ? Log::LOGTYPEV_WARNING : std::atoi(stLogDesc[0].c_str());

    switch(nLogType){
        case Log::LOGTYPEV_DEBUG:
            {
                g_Log->AddLog(stLogDesc, "%s", szLog.c_str());
                return;
            }
        default:
            {
                // flush the log window
                // make LOGTYPEV_FATAL be seen before process crash
                {
                    std::lock_guard<std::mutex> stLockGuard(m_LogLock);
                    m_LogBuf.push_back((char)(nLogType));
                    m_LogBuf.insert(m_LogBuf.end(), szLog.c_str(), szLog.c_str() + std::strlen(szLog.c_str()) + 1);
                }
                NotifyGUI("FlushBrowser");

                g_Log->AddLog(stLogDesc, "%s", szLog.c_str());
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

    std::string szLog;
    bool bError = false;
    {
        va_list ap;
        va_start(ap, szLogFormat);

        try{
            szLog = str_vprintf(szLogFormat, ap);
        }catch(const std::exception &e){
            bError = true;
            szLog = str_printf("Exception caught in MonoServer::AddCWLog(CWID = %" PRIu32 ", \"%s\", ...): %s", nCWID, szLogFormat, e.what());
        }

        va_end(ap);
    }

    if(bError){
        AddLog(LOGTYPE_WARNING, "%s", szLog.c_str());
    }

    if(bError){
        nLogType = Log::LOGTYPEV_WARNING;
    }
    fnCWRecordLog(nCWID, nLogType, szPrompt, szLog.c_str());
}

void MonoServer::CreateDefaultDatabase()
{
    if(std::strcmp(g_DBPodN->DBEngine(), "mysql") == 0){

    }

    else if(std::strcmp(g_DBPodN->DBEngine(), "sqlite3") == 0){
        const char *szSQLCmdList[]
        {
            u8"create table tbl_account("
                "fld_id       integer not null primary key autoincrement,"
                "fld_account  char(32) not null,"
                "fld_password char(32) not null)",

            u8"insert into tbl_account(fld_account, fld_password) values"
                "(\'test\',   \'123456\'),"
                "(\'test0\',  \'123456\'),"
                "(\'test1\',  \'123456\')",

            u8"create table tbl_dbid("
                "fld_dbid      integer not null primary key autoincrement,"
                "fld_id        int unsigned not null,"
                "fld_name      varchar(32)  not null,"
                "fld_mapname   varchar(32)  not null,"
                "fld_mapx      int unsigned not null,"
                "fld_mapy      int unsigned not null,"
                "fld_exp       int unsigned not null,"
                "fld_gold      int unsigned not null,"
                "fld_level     int unsigned not null,"
                "fld_jobid     int unsigned not null,"
                "fld_direction int unsigned not null)",

            u8"insert into tbl_dbid(fld_id, fld_name, fld_mapname, fld_mapx, fld_mapy, fld_exp, fld_gold, fld_level, fld_jobid, fld_direction) values"
                "(1, \'亚当\', \'道馆\',   405, 120, 0, 0, 1, 1, 1),"
                "(2, \'夏娃\', \'比奇省\', 441, 381, 0, 0, 1, 1, 1),"
                "(3, \'逗逼\', \'比奇省\', 440, 381, 0, 0, 1, 1, 1)",
        };

        auto pDBHDR = g_DBPodN->CreateDBHDR();
        for(size_t nIndex = 0; nIndex < std::extent<decltype(szSQLCmdList)>::value; ++nIndex){
            pDBHDR->QueryResult(szSQLCmdList[nIndex]);
        }
        AddLog(LOGTYPE_INFO, "Create default sqlite3 database done");
    }
}

void MonoServer::CreateDBConnection()
{
    if(std::strcmp(g_DatabaseConfigureWindow->SelectedDBEngine(), "mysql") == 0){
        g_DBPodN->LaunchMySQL(
                g_DatabaseConfigureWindow->DatabaseIP(),
                g_DatabaseConfigureWindow->UserName(),
                g_DatabaseConfigureWindow->Password(),
                g_DatabaseConfigureWindow->DatabaseName(),
                g_DatabaseConfigureWindow->DatabasePort());
        AddLog(LOGTYPE_INFO, "Connect to MySQL Database (%s:%d) successfully",
                g_DatabaseConfigureWindow->DatabaseIP(),
                g_DatabaseConfigureWindow->DatabasePort());
    }

    else if(std::strcmp(g_DatabaseConfigureWindow->SelectedDBEngine(), "sqlite3") == 0){
        g_DBPodN->LaunchSQLite3(g_DatabaseConfigureWindow->DatabaseName());
        AddLog(LOGTYPE_INFO, "Connect to SQLite3 Database (%s) successfully", g_DatabaseConfigureWindow->DatabaseName());

        if(!g_DBPodN->CreateDBHDR()->QueryResult("select name from sqlite_master where type=\'table\'")){
            CreateDefaultDatabase();
        }
    }

    else{
        AddLog(LOGTYPE_WARNING, "No database started");
        Restart();
    }
}

void MonoServer::LoadMapBinDBN()
{
    std::string szMapPath = g_ServerConfigureWindow->GetMapPath();

    if(!g_MapBinDBN->Load(szMapPath.c_str())){
        AddLog(LOGTYPE_WARNING, "Failed to load mapbindbn");
        Restart();
    }
}

void MonoServer::StartServiceCore()
{
    g_ActorPool->Launch();

    m_ServiceCore = new ServiceCore();
    m_ServiceCore->Activate();
}

void MonoServer::StartNetwork()
{
    uint32_t nPort = g_ServerConfigureWindow->Port();
    if(!g_NetDriver->Launch(nPort, m_ServiceCore->UID())){
        AddLog(LOGTYPE_WARNING, "Failed to launch the network");
        Restart();
    }
}

void MonoServer::Launch()
{
    CreateDBConnection();
    LoadMapBinDBN();

    StartServiceCore();
    StartNetwork();
}

void MonoServer::PropagateException()
{
    // TODO
    // add multi-thread protection
    try{
        if(!std::current_exception()){
            throw std::runtime_error("Call MonoServer::PropagateException() without exception captured");
        }

        // we do have an exception
        // but may not be std::exception, nest it...
        std::throw_with_nested(std::runtime_error("Rethrow in MonoServer::PropagateException()"));
    }catch(...){
        // must have one exception...
        // now we are sure main thread will always capture an std::exception
        m_CurrException = std::current_exception();
        Fl::awake((void *)(uintptr_t)(2));
    }
}

void MonoServer::DetectException()
{
    if(m_CurrException){
        std::rethrow_exception(m_CurrException);
    }
}

void MonoServer::LogException(const std::exception &rstException)
{
    AddLog(LOGTYPE_WARNING, "%s", rstException.what());
    try{
        std::rethrow_if_nested(rstException);
    }catch(const std::exception &stNestedException){
        LogException(stNestedException);
    }catch(...){
        AddLog(LOGTYPE_WARNING, "%s", "Exception can't recongize, skipped...");
    }
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

bool MonoServer::AddMonster(uint32_t nMonsterID, uint32_t nMapID, int nX, int nY, bool bRandom)
{
    AMAddCharObject stAMACO;
    std::memset(&stAMACO, 0, sizeof(stAMACO));

    stAMACO.Type = TYPE_MONSTER;

    stAMACO.Common.MapID  = nMapID;
    stAMACO.Common.X      = nX;
    stAMACO.Common.Y      = nY;
    stAMACO.Common.Random = bRandom;

    stAMACO.Monster.MonsterID = nMonsterID;
    stAMACO.Monster.MasterUID = 0;
    AddLog(LOGTYPE_INFO, "Try to add monster, MonsterID = %d", nMonsterID);

    switch(auto stRMPK = SyncDriver().Forward(m_ServiceCore->UID(), {MPK_ADDCHAROBJECT, stAMACO}, 0, 0); stRMPK.Type()){
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
    switch(auto stRMPK = SyncDriver().Forward(MPK_QUERYMAPLIST, m_ServiceCore->UID()); stRMPK.Type()){
        case MPK_MAPLIST:
            {
                AMMapList stAMML;
                std::memcpy(&stAMML, stRMPK.Data(), sizeof(stAMML));

                std::vector<int> stMapList;
                for(size_t nIndex = 0; nIndex < std::extent<decltype(stAMML.MapList)>::value; ++nIndex){
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

        switch(auto stRMPK = SyncDriver().Forward(m_ServiceCore->UID(), {MPK_QUERYCOCOUNT, stAMQCOC}); stRMPK.Type()){
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
            g_MonoServer->FlushBrowser();
            continue;
        }
        
        if(false
                || stTokenList.front() == "flushcwbrowser"
                || stTokenList.front() == "FlushCWBrowser"
                || stTokenList.front() == "FLUSHCWBROWSER"){
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
                g_MainWindow->DeleteCommandWindow(nCWID);
            }
            continue;
        }

        // unsupported command here
        // print into the log file and not exit here
        {
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

            g_MainWindow->AddCWLog(nCWID, (int)(m_CWLogBuf[nCurrLoc + sizeof(nCWID)]), pInfo0, pInfo1);
            nCurrLoc += (sizeof(nCWID) + 1 + nInfoLen0 + 1 + nInfoLen1 + 1);
        }
        m_CWLogBuf.clear();
    }
}

uint32_t MonoServer::SleepEx(uint32_t nTick)
{
    auto stEnterTime = std::chrono::steady_clock::now();
    auto stExpectedTime = stEnterTime + std::chrono::milliseconds(nTick);

    if(nTick > 20){
        std::this_thread::sleep_for(std::chrono::milliseconds(nTick - 10));
    }

    while(true){
        if(auto stCurrTime = std::chrono::steady_clock::now(); stCurrTime >= stExpectedTime){
            return (uint32_t)(std::chrono::duration_cast<std::chrono::microseconds>(stCurrTime - stEnterTime).count());
        }else{
            std::this_thread::sleep_for((stExpectedTime - stCurrTime) / 2);
        }
    }
    return 0;
}

bool MonoServer::RegisterLuaExport(CommandLuaModule *pModule, uint32_t nCWID)
{
    if(true
            && pModule      // module to execute lua script
            && nCWID){      // command window id to echo all execution information

        // register command quit
        // exit current command window and free all related resource
        pModule->GetLuaState().set_function("quit", [this, nCWID]()
        {
            // 1. show exiting messages
            AddCWLog(nCWID, 0, "> ", "Command window is requested to exit now...");

            // 2. flush command windows
            //    we need to flush message before exit the command window
            //    otherwise next created command window may get them if it uses the same CWID
            NotifyGUI("FlushCWBrowser");
            NotifyGUI(std::string("ExitCW ") + std::to_string(nCWID));
        });

        // register command printLine
        // print one line in command window, won't add message to log system
        pModule->GetLuaState().set_function("printLine", [this, nCWID](sol::object stLogType, sol::object stPrompt, sol::object stLogInfo)
        {
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

        // register command countMonster(monsterID, mapID)
        pModule->GetLuaState().set_function("countMonster", [this, nCWID](int nMonsterID, int nMapID) -> int
        {
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
        pModule->GetLuaState().set_function("addMonster", [this, nCWID](int nMonsterID, int nMapID, sol::variadic_args stVariadicArgs) -> bool
        {
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

        // register command mapList
        // return a table (userData) to lua for ipairs() check
        pModule->GetLuaState().set_function("getMapIDList", [this](sol::this_state stThisLua)
        {
            return sol::make_object(sol::state_view(stThisLua), GetMapList());
        });

        // register command ``listAllMap"
        // this command call getMapIDList to get a table and print to CommandWindow
        pModule->GetLuaState().script(
            R"###( function listMap()                                                      )###""\n"
            R"###(     local mapNameTable = {}                                             )###""\n"
            R"###(     for k, v in ipairs(getMapIDList()) do                               )###""\n"
            R"###(         table.insert(mapNameTable, mapID2Name(v))                       )###""\n"
            R"###(     end                                                                 )###""\n"
            R"###(     return mapNameTable                                                 )###""\n"
            R"###( end                                                                     )###""\n");

        // register command ``help"
        // part-1: divide into two parts, part-1 create the table

        pModule->GetLuaState().script(
            R"###( g_HelpTable = {}                                                        )###""\n"
            R"###( g_HelpTable["listMap"] = "print all map indices to current window"      )###""\n");

        // part-2: make up the function to print the table entry
        pModule->GetLuaState().script(
            R"###( function help(queryKey)                                                 )###""\n"
            R"###(     if g_HelpTable[queryKey] then                                       )###""\n"
            R"###(         printLine(0, "> ", g_HelpTable[queryKey])                       )###""\n"
            R"###(     else                                                                )###""\n"
            R"###(         printLine(2, "> ", "No registered help information for input")  )###""\n"
            R"###(     end                                                                 )###""\n"
            R"###( end                                                                     )###""\n");
    }
    return false;
}

/*
 * =====================================================================================
 *
 *       Filename: monoserver.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 06/04/2017 17:39:13
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
    , m_StartTime()
    , m_MonsterGInfoRecord()
{
    m_StartTime = std::chrono::system_clock::now();

    // 3. initialization of monster ginfo record
    m_MonsterGInfoRecord[ 1] = { 1, 0X0015, 0, 0, 0};
    m_MonsterGInfoRecord[10] = {10, 0X009F, 0, 0, 0};
}

void MonoServer::AddLog(const std::array<std::string, 4> &stLogDesc, const char *szLogFormat, ...)
{
    int nLogSize = 0;
    int nLogType = std::atoi(stLogDesc[0].c_str());

    auto fnRecordLog = [this, &stLogDesc](int nLogType, const char *szLogInfo){
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
                    Fl::awake((void *)(uintptr_t)(2));
                    break;
                }
        }
    };

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
        void Handler(const void *, const Theron::uint32_t, const Theron::Address stFromAddress)
        {
            // we lost the information that which actor it tried to send
            SyncDriver().Forward({MPK_BADACTORPOD}, stFromAddress);
        }
    }stFallbackHandler;

    extern Theron::Framework *g_Framework;
    g_Framework->SetFallbackHandler(&stFallbackHandler, &FrameworkFallbackHandler::Handler);
}

void MonoServer::CreateServiceCore()
{
    delete m_ServiceCore;
    m_ServiceCore = new ServiceCore();
    m_ServiceCore->Activate();
}

void MonoServer::StartNetwork()
{
    extern NetPodN *g_NetPodN;
    extern ServerConfigureWindow *g_ServerConfigureWindow;

    uint32_t nPort = g_ServerConfigureWindow->Port();
    if(g_NetPodN->Launch(nPort, m_ServiceCore->GetAddress())){
        AddLog(LOGTYPE_FATAL, "Failed to launch the network");
        Restart();
    }
}

void MonoServer::Launch()
{
    CreateDBConnection();
    LoadMonsterRecord();
    RegisterAMFallbackHandler();

    CreateServiceCore();
    StartNetwork();

    extern EventTaskHub *g_EventTaskHub;
    g_EventTaskHub->Launch();

    AddMonster(10, 1, 19, 19);
    AddMonster(10, 2,  8, 18);

    // AddMonster( 1, 2,  9, 18);
    // AddMonster( 1, 2, 10, 18);
    // AddMonster( 1, 2, 10, 19);
    // AddMonster( 1, 1, 19, 18);
    // AddMonster( 1, 1, 17, 15);
    // AddMonster( 1, 1, 16, 18);
    // AddMonster( 1, 1, 15, 17);
    // AddMonster( 1, 1, 16, 16);
    // AddMonster( 1, 1, 11, 21);
    // AddMonster( 1, 1, 20, 19);
    // AddMonster( 1, 1, 20, 20);
    // AddMonster( 1, 1, 20, 21);
    // AddMonster( 1, 1, 21, 21);
    AddMonster(10, 1,  8, 21);
    // AddMonster(10, 1,  8, 22);
    // AddMonster(10, 1,  9, 21);
    // AddMonster(10, 1,  9, 22);
    // AddMonster(10, 1,  9, 23);
    // AddMonster(10, 1,  9, 24);
    // AddMonster(10, 1, 14, 16);
    // AddMonster(10, 1, 14, 17);
    // AddMonster(10, 1, 14, 18);
    // AddMonster(10, 1, 14, 19);
    // AddMonster(10, 1, 14, 20);
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
    Fl::awake((void *)(uintptr_t)(1));
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

// request to add a new monster, this function won't expect a response
//
// TODO & TBD: why I won't put a response here? it will introduce huge complexity
// for me if this function can be informed with success or failure. i.e.
// 1. the map is invalid currently
// 2. the coresponding RM is invalid currently
//
// the I need two-hop response callback, and if map is valid but RM address is
// PENDING, then I have to make a anomyous trigger to add the monster in, and
// respond here we ask for it.
//
// but if no response, we never know this function is successful or not because
// we won't store monster (UID, AddTime)
void MonoServer::AddMonster(uint32_t nMonsterID, uint32_t nMapID, int nX, int nY)
{
    AMAddCharObject stAMACO;
    stAMACO.Type = TYPE_MONSTER;

    stAMACO.Common.MapID  = nMapID;
    stAMACO.Common.X      = nX;
    stAMACO.Common.Y      = nY;

    stAMACO.Monster.MonsterID = nMonsterID;
    AddLog(LOGTYPE_INFO, "add monster, MonsterID = %d", nMonsterID);

    MessagePack stRMPK;
    SyncDriver().Forward({MPK_ADDCHAROBJECT, stAMACO}, m_ServiceCore->GetAddress(), &stRMPK);
    switch(stRMPK.Type()){
        case MPK_OK:
            {
                AddLog(LOGTYPE_INFO, "add monster: adding succeeds");
                break;
            }
        case MPK_ERROR:
            {
                AddLog(LOGTYPE_WARNING, "add monster: operation failed");
                break;
            }
        default:
            {
                AddLog(LOGTYPE_WARNING, "unsupported message: %s", stRMPK.Name());
                break;
            }
    }
}

std::vector<uint32_t> MonoServer::GetActiveMapList()
{
    MessagePack stRMPK;
    SyncDriver().Forward(MPK_QUERYMAPLIST, m_ServiceCore->GetAddress(), &stRMPK);
    switch(stRMPK.Type()){
        case MPK_MAPLIST:
            {
                AMMapList stAMML;
                std::memcpy(&stAMML, stRMPK.Data(), sizeof(stAMML));

                std::vector<uint32_t> stMapList;
                for(size_t nIndex = 0; nIndex < sizeof(stAMML.MapList) / sizeof(stAMML.MapList[0]); ++nIndex){
                    if(stAMML.MapList[nIndex]){
                        stMapList.push_back(stAMML.MapList[nIndex]);
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

std::vector<uint32_t> MonoServer::GetValidMonsterList(uint32_t)
{
    return {1};
}

int MonoServer::GetValidMonsterCount(int, int)
{
    return 1;
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
        auto &rstRecord = m_UIDArray[nUID % m_UIDArray.size()];
        {
            std::lock_guard<std::mutex> stLockGuard(rstRecord.Lock);
            auto stFind = rstRecord.Record.find(nUID);
            if(stFind != rstRecord.Record.end()){
                if(stFind->second){
                    if(stFind->second->UID() == nUID){
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
                        auto bActive   = stFind->second->ClassFrom<ActiveObject>();
                        auto stAddress = bActive ? ((ActiveObject *)(stFind->second))->GetAddress() : Theron::Address::Null();
                        return {nUID, stAddress, stFind->second->ClassEntry()};
                    }else{
                        AddLog(LOGTYPE_WARNING, "UIDArray mismatch: UID = (%" PRIu32 ", %" PRIu32 ")", nUID, stFind->second->UID());
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
    return UIDRecord(0, Theron::Address::Null(), stNullEntry);
}

bool MonoServer::RegisterLuaExport(ServerLuaModule *pModule, CommandWindow *pWindow)
{
    if(true
            && pModule      // module to execute lua script
            && pWindow){    // command window to echo all execution information

        // initialization before registration
        pModule->script(R"()");

        // register command printLine
        // print one line in command window
        // won't add message to log system, use addLog instead
        pModule->set_function("printLine", [pWindow](sol::object stLogType, sol::object stPrompt, sol::object stLogInfo){
            // use sol::object to accept arguments
            // otherwise for follow code it throws exception for type unmatch
            //      lua["f"] = [](int a){ return a; };
            //      lua.script("print f(\"hello world\")")
            // program crashes with exception.what() : expecting int, string provided
            if(true
                    && stLogType.is<int>()
                    && stPrompt.is<std::string>()
                    && stLogInfo.is<std::string>()){
                pWindow->AddLog(stLogType.as<int>(), stPrompt.as<std::string>().c_str(), stLogInfo.as<std::string>().c_str());
                return;
            }

            // invalid argument provided
            pWindow->AddLog(2, ">>> ", "printLine(LogType: int, Prompt: string, LogInfo: string)");
        });

        // register command addLog
        // add to system log file and main window history window
        pModule->set_function("addLog", [this, pWindow](sol::object stLogType, sol::object stLogInfo){
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
            pWindow->AddLog(2, ">>> ", "addLog(LogType: int, LogInfo: string)");
        });

        // register command mapList
        // get a list for all active maps
        // return a table (userData) to lua for ipairs() check
        pModule->set_function("mapList", [this](sol::this_state stThisLua){
            return sol::make_object(sol::state_view(stThisLua), GetActiveMapList());
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

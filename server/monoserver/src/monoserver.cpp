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
#include "jobf.hpp"
#include "dbpod.hpp"
#include "totype.hpp"
#include "taskhub.hpp"
#include "message.hpp"
#include "monster.hpp"
#include "mapbindb.hpp"
#include "fflerror.hpp"
#include "actorpool.hpp"
#include "syncdriver.hpp"
#include "mainwindow.hpp"
#include "monoserver.hpp"
#include "dispatcher.hpp"
#include "servicecore.hpp"
#include "eventtaskhub.hpp"
#include "commandwindow.hpp"

extern Log *g_log;
extern DBPod *g_dbPod;
extern MapBinDB *g_mapBinDB;
extern ActorPool *g_actorPool;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;
extern MainWindow *g_mainWindow;
extern ServerConfigureWindow *g_serverConfigureWindow;

MonoServer::MonoServer()
    : m_logLock()
    , m_logBuf()
    , m_serviceCore(nullptr)
    , m_currException()
    , m_hrtimer()
{}

void MonoServer::addLog(const std::array<std::string, 4> &logDesc, const char *format, ...)
{
    std::string s;
    str_format(format, s);
    const int logType = std::atoi(logDesc[0].c_str());

    switch(logType){
        case Log::LOGTYPEV_DEBUG:
            {
                g_log->addLog(logDesc, "%s", s.c_str());
                return;
            }
        default:
            {
                // flush the log window
                // make LOGTYPEV_FATAL be seen before process crash
                {
                    std::lock_guard<std::mutex> lockGuard(m_logLock);
                    m_logBuf.push_back((char)(logType));
                    m_logBuf.insert(m_logBuf.end(), s.c_str(), s.c_str() + std::strlen(s.c_str()) + 1);
                }
                notifyGUI("FlushBrowser");

                g_log->addLog(logDesc, "%s", s.c_str());
                return;
            }
    }
}

void MonoServer::addCWLog(uint32_t nCWID, int nLogType, const char *szPrompt, const char *szLogFormat, ...)
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
            notifyGUI("FlushCWBrowser");
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
            szLog = str_printf("Exception caught in MonoServer::addCWLog(CWID = %" PRIu32 ", \"%s\", ...): %s", nCWID, szLogFormat, e.what());
        }

        va_end(ap);
    }

    if(bError){
        addLog(LOGTYPE_WARNING, "%s", szLog.c_str());
    }

    if(bError){
        nLogType = Log::LOGTYPEV_WARNING;
    }
    fnCWRecordLog(nCWID, nLogType, szPrompt, szLog.c_str());
}

bool MonoServer::hasDatabase() const
{
    return g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table')###").executeStep();
}

bool MonoServer::hasCharacter(const char *charName) const
{
    if(!str_haschar(charName)){
        throw fflerror("invalid char name: %s", to_cstr(charName));
    }
    return g_dbPod->createQuery(u8R"###(select fld_dbid from tbl_dbid where fld_name = '%s')###", charName).executeStep();
}

int MonoServer::createAccount(const char *id, const char *password)
{
    if(!(str_haschar(id) && (str_haschar(password)))){
        throw fflerror("bad account: id = %s, password = %s", to_cstr(id), to_cstr(password));
    }

    if(!hasDatabase()){
        throw fflerror("no available database");
    }

    if(g_dbPod->createQuery(u8R"###(select fld_dbid from tbl_account where fld_account ='%s')###", id).executeStep()){
        return CAERR_EXIST;
    }

    g_dbPod->exec(u8R"###(insert into tbl_account(fld_account, fld_password) values ('%s', '%s'))###", id, password);
    return CAERR_NONE;
}

bool MonoServer::createAccountCharacter(const char *id, const char *charName, bool gender, const char *job)
{
    if(!(str_haschar(id) && str_haschar(charName) && str_haschar(job))){
        throw fflerror("invalid argumets: id = %s, charName = %s, gender = %s, job = %s", to_cstr(id), to_cstr(charName), gender ? "male" : "female", to_cstr(job));
    }

    if(!hasDatabase()){
        throw fflerror("no available database");
    }

    if(hasCharacter(charName)){
        return false;
    }

    auto queryAccount = g_dbPod->createQuery(u8R"###(select fld_dbid from tbl_account where fld_account ='%s')###", id);
    if(!queryAccount.executeStep()){
        return false;
    }

    const auto dbid = check_cast<uint32_t, unsigned>(queryAccount.getColumn("fld_dbid"));
    const auto jobList = jobf::getJobList(job);

    if(jobList.empty()){
        throw fflerror("invalid job string: %s", to_cstr(job));
    }

    const auto [mapName, mapx, mapy] = [&jobList]() -> std::tuple<const char8_t *, int, int>
    {
        switch(jobList.front()){
            case JOB_MAGE:
            case JOB_WARRIOR:
                {
                    return {u8"比奇省", 441, 381};
                }
            case JOB_TAOIST:
                {
                    return {u8"道馆", 405, 120};
                }
            default:
                {
                    throw bad_reach();
                }
        }
    }();

    g_dbPod->exec
    (
        u8R"###( insert into tbl_dbid(fld_dbid, fld_name, fld_job, fld_mapname, fld_mapx, fld_mapy, fld_gender) )###"
        u8R"###( values                                                                                         )###"
        u8R"###(     (%llu, '%s', '%s', '%s', %d, %d, %d);                                                      )###",

        to_llu(dbid),
        charName,
        to_cstr(jobf::getJobString(jobList)),
        to_cstr(mapName),
        mapx,
        mapy,
        (int)(gender)
    );
    return true;
}

void MonoServer::createDefaultDatabase()
{
    const char8_t *defSQLCmdList[]
    {
        u8R"###( create table tbl_account(                                        )###"
        u8R"###(     fld_dbid        integer  not null primary key autoincrement, )###"
        u8R"###(     fld_account     char(64) not null,                           )###"
        u8R"###(     fld_password    char(64) not null                            )###"
        u8R"###( );                                                               )###",

        u8R"###( insert into tbl_account(fld_dbid, fld_account, fld_password)     )###"
        u8R"###( values                                                           )###"
        u8R"###(     (1, 'admin', 'admin');                                       )###",

        u8R"###( create table tbl_dbid(                                           )###"
        u8R"###(     fld_dbid        integer      not null primary key,           )###"
        u8R"###(     fld_name        varchar(32)  not null,                       )###"
        u8R"###(     fld_namecolor   int unsigned default 0,                      )###"
        u8R"###(     fld_job         varchar(32)  not null,                       )###"
        u8R"###(     fld_mapname     varchar(32)  not null,                       )###"
        u8R"###(     fld_mapx        int unsigned not null,                       )###"
        u8R"###(     fld_mapy        int unsigned not null,                       )###"
        u8R"###(     fld_hp          int unsigned default 10,                     )###"
        u8R"###(     fld_mp          int unsigned default 10,                     )###"
        u8R"###(     fld_exp         int unsigned default 0,                      )###"
        u8R"###(     fld_gold        int unsigned default 10000,                  )###"
        u8R"###(     fld_gender      boolean not null,                            )###"
        u8R"###(     fld_hair        int unsigned default 0,                      )###"
        u8R"###(     fld_haircolor   int unsigned default 0                       )###"
        u8R"###( );                                                               )###",

        u8R"###( create table tbl_belt(                                           )###"
        u8R"###(     fld_dbid        int unsigned not null,                       )###"
        u8R"###(     fld_belt        int unsigned not null,                       )###"
        u8R"###(                                                                  )###"
        u8R"###(     fld_itemid      int unsigned not null,                       )###"
        u8R"###(     fld_count       int unsigned not null,                       )###"
        u8R"###(                                                                  )###"
        u8R"###(     foreign key (fld_dbid) references tbl_dbid(fld_dbid),        )###"
        u8R"###(     primary key (fld_dbid, fld_belt)                             )###"
        u8R"###( );                                                               )###",

        u8R"###( create table tbl_wear(                                           )###"
        u8R"###(     fld_dbid        int unsigned not null,                       )###"
        u8R"###(     fld_wear        int unsigned not null,                       )###"
        u8R"###(                                                                  )###"
        u8R"###(     fld_itemid      int unsigned not null,                       )###"
        u8R"###(     fld_count       int unsigned not null,                       )###"
        u8R"###(     fld_duration    int unsigned not null,                       )###"
        u8R"###(     fld_extattrlist blob null default (x''),                     )###"
        u8R"###(                                                                  )###"
        u8R"###(     foreign key (fld_dbid) references tbl_dbid(fld_dbid),        )###"
        u8R"###(     primary key (fld_dbid, fld_wear)                             )###"
        u8R"###( );                                                               )###",

        u8R"###( create table tbl_inventory(                                      )###"
        u8R"###(     fld_dbid        int unsigned not null,                       )###"
        u8R"###(     fld_itemid      int unsigned not null,                       )###"
        u8R"###(     fld_seqid       int unsigned not null,                       )###"
        u8R"###(     fld_count       int unsigned not null,                       )###"
        u8R"###(     fld_duration    int unsigned not null,                       )###"
        u8R"###(     fld_extattrlist blob         not null,                       )###"
        u8R"###(                                                                  )###"
        u8R"###(     foreign key (fld_dbid) references tbl_dbid(fld_dbid),        )###"
        u8R"###(     primary key (fld_dbid, fld_itemid, fld_seqid)                )###"
        u8R"###( );                                                               )###",
    };

    {
        auto dbTrans = g_dbPod->createTransaction();
        for(const auto sqlCmd: defSQLCmdList){
            g_dbPod->exec(sqlCmd);
        }
        dbTrans.commit();
    }

    createAccount("test", "123456");
    createAccount("good", "123456");
    createAccount("id_1", "123456");
    createAccount("id_2", "123456");

    createAccountCharacter("test", to_cstr(u8"亚当"),  true, to_cstr(u8"道士|法师"));
    createAccountCharacter("good", to_cstr(u8"夏娃"), false, to_cstr(u8"战士|法师"));
    createAccountCharacter("id_1", to_cstr(u8"逗逼"),  true, to_cstr(u8"法师"));
    createAccountCharacter("id_2", to_cstr(u8"搞笑"),  true, to_cstr(u8"法师"));

    addLog(LOGTYPE_INFO, "Create default sqlite3 database done");
}

void MonoServer::CreateDBConnection()
{
    const char *dbName = "mir2x.db3";
    g_dbPod->launch(dbName);

    if(!hasDatabase()){
        createDefaultDatabase();
    }
    addLog(LOGTYPE_INFO, "Connect to database %s successfully", dbName);
}

void MonoServer::LoadMapBinDB()
{
    std::string szMapPath = g_serverConfigureWindow->GetMapPath();

    if(!g_mapBinDB->Load(szMapPath.c_str())){
        throw fflerror("Failed to load mapbindb");
    }
}

void MonoServer::StartServiceCore()
{
    g_actorPool->launchPool();

    m_serviceCore = new ServiceCore();
    m_serviceCore->activate();
}

void MonoServer::StartNetwork()
{
    uint32_t nPort = g_serverConfigureWindow->Port();
    if(!g_netDriver->Launch(nPort, m_serviceCore->UID())){
        throw fflerror("Failed to launch the network");
    }
}

void MonoServer::Launch()
{
    CreateDBConnection();
    LoadMapBinDB();

    StartServiceCore();
    StartNetwork();
}

void MonoServer::propagateException() noexcept
{
    // TODO
    // add multi-thread protection
    try{
        if(!std::current_exception()){
            addLog(LOGTYPE_WARNING, "call MonoServer::propagateException() without exception captured");
            return;
        }

        // we do have an exception
        // but may not be std::exception, nest it...
        std::throw_with_nested(fflerror("rethrow in MonoServer::propagateException()"));
    }
    catch(...){
        // must have one exception...
        // now we are sure main thread will always capture an std::exception
        m_currException = std::current_exception();
        Fl::awake((void *)(uintptr_t)(2));
    }
}

void MonoServer::checkException()
{
    if(m_currException){
        std::rethrow_exception(m_currException);
    }
}

void MonoServer::logException(const std::exception &except, std::string *strPtr) noexcept
{
    addLog(LOGTYPE_WARNING, "%s", except.what());
    try{
        if(strPtr){
            strPtr->assign(except.what());
        }
        std::rethrow_if_nested(except);
    }
    catch(const std::exception &nextedExcept){
        logException(nextedExcept, strPtr);
    }
    catch(...){
        addLog(LOGTYPE_WARNING, "Can't recognize exception, skipped...");
    }
}

void MonoServer::restart(const std::string &msg)
{
    // TODO: FLTK multi-threading support is weak, see:
    // http://www.fltk.org/doc-1.3/advanced.html#advanced_multithreading

    // Fl::awake() will send message to main loop
    // define the main loop to call exit(0) when pass 1 to main thread

    // this function itself can be called from
    //   1. main loop
    //   2. child thread

    if(msg.empty()){
        notifyGUI("Restart");
    }

    else if(msg.find('\n') != std::string::npos){
        notifyGUI("Restart\nfatal message contains \\n, ignored");
    }

    else{
        notifyGUI(std::string("Restart") + "\n" + msg);
    }
}

bool MonoServer::addMonster(uint32_t monsterID, uint32_t mapID, int x, int y, bool strictLoc)
{
    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type = UID_MON;
    amACO.x = x;
    amACO.y = y;
    amACO.mapID = mapID;
    amACO.strictLoc = strictLoc;

    amACO.monster.monsterID = monsterID;
    amACO.monster.masterUID = 0;
    addLog(LOGTYPE_INFO, "Try to add monster, monsterID = %llu", to_llu(monsterID));

    switch(auto stRMPK = SyncDriver().forward(m_serviceCore->UID(), {AM_ADDCHAROBJECT, amACO}, 0, 0); stRMPK.type()){
        case AM_OK:
            {
                addLog(LOGTYPE_INFO, "Add monster succeeds");
                return true;
            }
        case AM_ERROR:
            {
                addLog(LOGTYPE_WARNING, "Add monster failed");
                return false;
            }
        default:
            {
                addLog(LOGTYPE_WARNING, "Unsupported message: %s", mpkName(stRMPK.type()));
                return false;
            }
    }
}

bool MonoServer::addNPChar(uint16_t npcID, uint32_t mapID, int x, int y, bool strictLoc)
{
    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type = UID_NPC;
    amACO.x = x;
    amACO.y = y;
    amACO.mapID = mapID;
    amACO.strictLoc = strictLoc;

    amACO.NPC.NPCID = npcID;
    addLog(LOGTYPE_INFO, "Try to add NPC, NPCID = %llu", to_llu(npcID));

    switch(auto stRMPK = SyncDriver().forward(m_serviceCore->UID(), {AM_ADDCHAROBJECT, amACO}, 0, 0); stRMPK.type()){
        case AM_OK:
            {
                addLog(LOGTYPE_INFO, "Add NPC succeeds");
                return true;
            }
        case AM_ERROR:
            {
                addLog(LOGTYPE_WARNING, "Add NPC failed");
                return false;
            }
        default:
            {
                throw fflerror("Unsupported message: %s", mpkName(stRMPK.type()));
            }
    }
}

std::vector<int> MonoServer::GetMapList()
{
    switch(auto stRMPK = SyncDriver().forward(m_serviceCore->UID(), AM_QUERYMAPLIST); stRMPK.type()){
        case AM_MAPLIST:
            {
                AMMapList amML;
                std::memcpy(&amML, stRMPK.data(), sizeof(amML));

                std::vector<int> stMapList;
                for(size_t nIndex = 0; nIndex < std::extent<decltype(amML.MapList)>::value; ++nIndex){
                    if(amML.MapList[nIndex]){
                        stMapList.push_back((int)(amML.MapList[nIndex]));
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

        AMQueryCOCount amQCOC;
        std::memset(&amQCOC, 0, sizeof(amQCOC));

        amQCOC.mapID                = (uint32_t)(nMapID);
        amQCOC.Check.Monster        = true;
        amQCOC.CheckParam.MonsterID = (uint32_t)(nMonsterID);

        switch(auto stRMPK = SyncDriver().forward(m_serviceCore->UID(), {AM_QUERYCOCOUNT, amQCOC}); stRMPK.type()){
            case AM_COCOUNT:
                {
                    AMCOCount amCOC;
                    std::memcpy(&amCOC, stRMPK.data(), sizeof(amCOC));

                    // may overflow
                    // should put some check here
                    return sol::optional<int>((int)(amCOC.Count));
                }
            case AM_ERROR:
            default:
                {
                    break;
                }
        }
    }
    return sol::optional<int>();
}

void MonoServer::notifyGUI(std::string notifStr)
{
    if(!notifStr.empty()){
        {
            std::lock_guard<std::mutex> lockGuard(m_notifyGUILock);
            m_notifyGUIQ.push(notifStr);
        }
        Fl::awake((void *)(uintptr_t)(1));
    }
}

void MonoServer::parseNotifyGUIQ()
{
    const auto fnGetTokenList = [](const std::string &cmdStr) -> std::deque<std::string>
    {
        size_t currLoc = 0;
        std::deque<std::string> tokenList;

        while(currLoc <= cmdStr.size()){
            const auto loc0 = cmdStr.find_first_not_of("\n", currLoc);
            const auto loc1 = cmdStr.find_first_of    ("\n", currLoc);

            if(loc0 == std::string::npos){
                // done parsing
                // there are no more tokens in the string
                break;
            }

            else if(loc1 == std::string::npos){
                // last one
                // loc0 is OK
                // loc1 reaches the end
                // this is the last token in the list
                tokenList.push_back(cmdStr.substr(loc0));
                break;
            }

            else if(loc0 < loc1){
                // match cases
                // make sure loc0 < loc1 to avoid cases like: "    OK 1"
                tokenList.push_back(cmdStr.substr(loc0, loc1 - loc0));
                currLoc = loc1;
                continue;
            }

            else{
                // case for loc0 > loc1, no equal here
                // cases like "   OK 1", take no token and move forward
                currLoc = loc0;
                continue;
            }
        }
        return tokenList;
    };

    while(true){
        std::string currNotify;
        {
            std::lock_guard<std::mutex> lockGuard(m_notifyGUILock);
            if(m_notifyGUIQ.empty()){
                break;
            }

            currNotify = std::move(m_notifyGUIQ.front());
            m_notifyGUIQ.pop();
        }

        const auto tokenList = fnGetTokenList(currNotify);
        if(tokenList.empty()){
            continue;
        }

        const auto fnCheckFront = [&tokenList](const std::vector<std::string> &keyList) -> bool
        {
            for(const auto &key: keyList){
                if(tokenList.front() == key){
                    return true;
                }
            }
            return false;
        };

        if(fnCheckFront({"exit", "Exit", "EXIT"})){
            std::exit(0);
            return;
        }

        if(fnCheckFront({"restart", "Restart", "RESTART"})){
            if(tokenList.size() == 1){
                fl_alert("Fatal error");
            }
            else{
                fl_alert("Fatal error: %s", tokenList.at(1).c_str());
            }
            std::exit(0);
            return;
        }

        if(fnCheckFront({"flushbrowser", "FlushBrowser", "FLUSHBROWSER"})){
            g_monoServer->FlushBrowser();
            continue;
        }

        if(fnCheckFront({"flushcwbrowser", "FlushCWBrowser", "FLUSHCWBROWSER"})){
            g_monoServer->FlushCWBrowser();
            continue;
        }

        if(fnCheckFront({"exitcw", "ExitCW", "EXITCW"})){
            const int cwid = [&tokenList]() -> int
            {
                try{
                    return std::stoi(tokenList.at(1));
                }catch(...){
                    //
                }
                return 0;
            }();

            if(cwid > 0){
                g_mainWindow->DeleteCommandWindow(cwid);
            }
            continue;
        }

        g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported notification: %s", tokenList.front().c_str());
    }
}

void MonoServer::FlushBrowser()
{
    std::lock_guard<std::mutex> stLockGuard(m_logLock);
    {
        auto nCurrLoc = (size_t)(0);
        while(nCurrLoc < m_logBuf.size()){
            g_mainWindow->addLog((int)(m_logBuf[nCurrLoc]), &(m_logBuf[nCurrLoc + 1]));
            nCurrLoc += (1 + 1 + std::strlen(&(m_logBuf[nCurrLoc + 1])));
        }
        m_logBuf.clear();
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

            g_mainWindow->addCWLog(nCWID, (int)(m_CWLogBuf[nCurrLoc + sizeof(nCWID)]), pInfo0, pInfo1);
            nCurrLoc += (sizeof(nCWID) + 1 + nInfoLen0 + 1 + nInfoLen1 + 1);
        }
        m_CWLogBuf.clear();
    }
}

uint64_t MonoServer::sleepExt(uint64_t tickCount)
{
    const auto enterTime = std::chrono::steady_clock::now();
    const auto excptTime = enterTime + std::chrono::milliseconds(tickCount);

    if(tickCount > 20){
        std::this_thread::sleep_for(std::chrono::milliseconds(tickCount - 10));
    }

    while(true){
        if(const auto currTime = std::chrono::steady_clock::now(); currTime >= excptTime){
            return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(currTime - enterTime).count());
        }
        else{
            std::this_thread::sleep_for((excptTime - currTime) / 2);
        }
    }
    return 0;
}

void MonoServer::regLuaExport(CommandLuaModule *pModule, uint32_t nCWID)
{
    if(!(pModule && nCWID)){
        throw fflerror("invalid argument: module = %p, window ID = %llu", to_cvptr(pModule), to_llu(nCWID));
    }

    // register command quit
    // exit current command window and free all related resource
    pModule->getLuaState().set_function("quit", [this, nCWID]()
    {
        // 1. show exiting messages
        addCWLog(nCWID, 0, "> ", "Command window is requested to exit now...");

        // 2. flush command windows
        //    we need to flush message before exit the command window
        //    otherwise next created command window may get them if it uses the same CWID
        notifyGUI("FlushCWBrowser");
        notifyGUI(std::string("ExitCW\n") + std::to_string(nCWID));
    });

    // register command printLine
    // print one line in command window, won't add message to log system
    pModule->getLuaState().set_function("printLine", [this, nCWID](sol::object stLogType, sol::object stPrompt, sol::object stLogInfo)
    {
        if(true
                && stLogType.is<int>()
                && stPrompt.is<std::string>()
                && stLogInfo.is<std::string>()){
            addCWLog(nCWID, stLogType.as<int>(), stPrompt.as<std::string>().c_str(), stLogInfo.as<std::string>().c_str());
            return;
        }

        // invalid argument provided
        addCWLog(nCWID, 2, ">>> ", "printLine(LogType: int, Prompt: string, LogInfo: string)");
    });

    // register command countMonster(monsterID, mapID)
    pModule->getLuaState().set_function("countMonster", [this, nCWID](int nMonsterID, int nMapID) -> int
    {
        auto nRet = GetMonsterCount(nMonsterID, nMapID).value_or(-1);
        if(nRet < 0){
            addCWLog(nCWID, 2, ">>> ", "countMonster(MonsterID: int, mapID: int) failed");
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
    pModule->getLuaState().set_function("addMonster", [this, nCWID](int nMonsterID, int nMapID, sol::variadic_args stVariadicArgs) -> bool
    {
        auto fnPrintUsage = [this, nCWID]()
        {
            addCWLog(nCWID, 2, ">>> ", "addMonster(MonsterID: int, mapID: int)");
            addCWLog(nCWID, 2, ">>> ", "addMonster(MonsterID: int, mapID: int, X: int, Y: int)");
            addCWLog(nCWID, 2, ">>> ", "addMonster(MonsterID: int, mapID: int, X: int, Y: int, Random: bool)");
        };

        std::vector<sol::object> stArgList(stVariadicArgs.begin(), stVariadicArgs.end());
        switch(stArgList.size()){
            case 0:
                {
                    return addMonster((uint32_t)(nMonsterID), (uint32_t)(nMapID), -1, -1, false);
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
                        return addMonster((uint32_t)(nMonsterID), (uint32_t)(nMapID), stArgList[0].as<int>(), stArgList[1].as<int>(), false);
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
                        return addMonster((uint32_t)(nMonsterID), (uint32_t)(nMapID), stArgList[0].as<int>(), stArgList[1].as<int>(), stArgList[2].as<bool>());
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

    pModule->getLuaState().set_function("addNPC", [this, nCWID](int npcID, int mapID, sol::variadic_args args) -> bool
    {
        const auto fnUsage = [this, nCWID]()
        {
            addCWLog(nCWID, 2, ">>> ", "addNPC(NPCID: int, mapID: int)");
            addCWLog(nCWID, 2, ">>> ", "addNPC(NPCID: int, mapID: int, X: int, Y: int)");
            addCWLog(nCWID, 2, ">>> ", "addNPC(NPCID: int, mapID: int, X: int, Y: int, Random: bool)");
        };

        const std::vector<sol::object> argList(args.begin(), args.end());
        switch(argList.size()){
            case 0:
                {
                    return addNPChar((uint16_t)(npcID), (uint32_t)(mapID), -1, -1, false);
                }
            case 2:
                {
                    if(argList[0].is<int>() && argList[1].is<int>()){
                        return addNPChar((uint32_t)(npcID), (uint32_t)(mapID), argList[0].as<int>(), argList[1].as<int>(), false);
                    }
                    break;
                }
            case 3:
                {
                    if(argList[0].is<int >() && argList[1].is<int >() && argList[2].is<bool>()){
                        return addNPChar((uint32_t)(npcID), (uint32_t)(mapID), argList[0].as<int>(), argList[1].as<int>(), argList[2].as<bool>());
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }

        fnUsage();
        return false;
    });

    // register command mapList
    // return a table (userData) to lua for ipairs() check
    pModule->getLuaState().set_function("getMapIDList", [this](sol::this_state stThisLua)
    {
        return sol::make_object(sol::state_view(stThisLua), GetMapList());
    });

    pModule->getLuaState().set_function("mapID2Name", [](int mapID) -> std::string
    {
        return to_cstr(DBCOM_MAPRECORD(mapID).name);
    });

    // register command ``listAllMap"
    // this command call getMapIDList to get a table and print to CommandWindow
    pModule->getLuaState().script(
        R"###( function listMap()                                                      )###""\n"
        R"###(     local mapNameTable = {}                                             )###""\n"
        R"###(     for k, v in ipairs(getMapIDList()) do                               )###""\n"
        R"###(         printLine(0, ".", tostring(v) .. " " .. mapID2Name(v))          )###""\n"
        R"###(     end                                                                 )###""\n"
        R"###( end                                                                     )###""\n");

    // register command ``help"
    // part-1: divide into two parts, part-1 create the table

    pModule->getLuaState().script(
        R"###( g_helpTable = {}                                                        )###""\n"
        R"###( g_helpTable["listMap"] = "print all map indices to current window"      )###""\n");

    // part-2: make up the function to print the table entry
    pModule->getLuaState().script(
        R"###( function help(queryKey)                                                 )###""\n"
        R"###(     if g_helpTable[queryKey] then                                       )###""\n"
        R"###(         printLine(0, "> ", g_helpTable[queryKey])                       )###""\n"
        R"###(     else                                                                )###""\n"
        R"###(         printLine(2, "> ", "No registered help information for input")  )###""\n"
        R"###(     end                                                                 )###""\n"
        R"###( end                                                                     )###""\n");
}

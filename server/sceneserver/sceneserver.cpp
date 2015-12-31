#include <algorithm>
#include "message.hpp"
#include "database.hpp"
#include "sceneserver.hpp"
#include "messagemanager.hpp"
#include "mainwindow.hpp"
#include "servermessagedef.hpp"
#include "serverconfigurewindow.hpp"
#include "newmir2map.hpp"
#include "addmonsterwindow.hpp"

SceneServer::SceneServer()
    : m_DBConnection(nullptr)
    , m_HumanThread(nullptr)
    // , m_MaxMonsterCount(5000)  // TODO put it in ServerConfigureWindow
    , m_MaxMonsterUID(0)
    , m_MonsterThread(nullptr)
    , m_UpdateTime(500)
    , m_Quit(false)
{
    extern NewMir2Map g_Map;
    m_Map = &g_Map;
}

SceneServer::~SceneServer()
{}

void SceneServer::Init()
{
}

bool SceneServer::LoadMap()
{
    extern ServerConfigureWindow *g_ServerConfigureWindow;
	extern MainWindow            *g_MainWindow;
	extern NewMir2Map             g_Map;

    std::string szTmpMapPath = g_ServerConfigureWindow->MapFullName();
    std::replace(szTmpMapPath.begin(), szTmpMapPath.end(), '\\', '/');
    g_Map.NewLoadMap((szTmpMapPath + "/desc.bin").c_str());

    if(g_Map.Valid()){
        g_MainWindow->AddLog(0, "Load map successfully");
        int nPos = szTmpMapPath.find_last_of('/');
        m_MapName = szTmpMapPath.substr(nPos + 1, szTmpMapPath.size() - nPos);
		return true;
    }else{
        extern ServerConfigureWindow *g_ServerConfigureWindow;
        char szInfo[128];
        sprintf(szInfo, "Fail to load map: %s", g_ServerConfigureWindow->MapFullName());
        g_MainWindow->AddLog(2, szInfo);
        return false;
    }
}

bool SceneServer::ConnectToGateServer()
{
	extern MainWindow *g_MainWindow;
    g_MainWindow->AddLog(0, "establishing connection to gate server");
    // GetMessageManager()->Stop();
    GetMessageManager()->Start();
	return true;
}

bool SceneServer::ConnectToDatebase()
{
    extern MainWindow *g_MainWindow;
    g_MainWindow->AddLog(0, "establishing connection to database");

    extern NetworkConfigureWindow *g_NetworkConfigureWindow;
    delete m_DBConnection;
    m_DBConnection = new DBConnection(
            g_NetworkConfigureWindow->DatabaseIP(),
            g_NetworkConfigureWindow->UserName(),
            g_NetworkConfigureWindow->Password(),
            g_NetworkConfigureWindow->DatabaseName(),
            g_NetworkConfigureWindow->DatabasePort());

    char szInfo[128];
    if(m_DBConnection->Valid()){
        std::sprintf(szInfo, "Connect to database (%s:%d) successfully", 
                g_NetworkConfigureWindow->DatabaseIP(),
                g_NetworkConfigureWindow->DatabasePort());
        g_MainWindow->AddLog(0, szInfo);
		return true;
    }else{
        std::sprintf(szInfo, "Can't connect to Database (%s:%d)", 
                g_NetworkConfigureWindow->DatabaseIP(),
                g_NetworkConfigureWindow->DatabasePort());
        g_MainWindow->AddLog(2, szInfo);
		return false;
    }
}

void SceneServer::Launch()
{
    extern MainWindow *g_MainWindow;
    g_MainWindow->AddLog(0, "Launching scene server");

    if(true
            && LoadMap()
            && ConnectToDatebase()
            && ConnectToGateServer()
      ){
        UpdateValidMonsterList();
		StartServerLogicThread();
    }
}

bool SceneServer::StartServerLogicThread()
{

    m_StartTime     = std::chrono::system_clock::now();
    m_MonsterThread = new std::thread([this](){ MonsterThread(); });
    m_HumanThread   = new std::thread([this](){ HumanThread();   });

    return true;
}

bool SceneServer::RequestQuit()
{
    return m_Quit;
}

void SceneServer::MonsterThread()
{
    while(!RequestQuit()){

		// printf("monster thread\n");

        uint32_t nUpdateStartTime = GetTimeMS();

        {
            std::lock_guard<std::mutex> stGuard(m_MonsterListMutex);
            TryAddMonster();
            for(auto pMonster: m_MonsterList){
                pMonster->Update();
            }
        }

        uint32_t nUpdateStopTime = GetTimeMS();

        if(nUpdateStopTime - nUpdateStartTime < m_UpdateTime){
            DelayMS(m_UpdateTime - (nUpdateStopTime - nUpdateStartTime));
        }
    }
}

void SceneServer::HumanThread()
{
    while(!RequestQuit()){

		// printf("human thread: %d\n", GetTimeMS());

        uint32_t nUpdateStartTime = GetTimeMS();

        {
            std::lock_guard<std::mutex> stGuard(m_HumanListMutex);
            for(auto pHuman: m_HumanList){
                pHuman->Update();
            }
        }

        uint32_t nUpdateStopTime = GetTimeMS();

        if(nUpdateStopTime - nUpdateStartTime < m_UpdateTime){
            DelayMS(m_UpdateTime - (nUpdateStopTime - nUpdateStartTime));
        }
    }
}

uint32_t SceneServer::GetTimeMS()
{
    std::chrono::duration<double, std::ratio<1,1000>> stDiff
        = std::chrono::system_clock::now() - m_StartTime;
    return (uint32_t)std::lround(stDiff.count());
}

void SceneServer::DelayMS(uint32_t nMS)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(nMS));
}

void SceneServer::UpdateValidMonsterList()
{
    extern AddMonsterWindow *g_AddMonsterWindow;

    if(!m_DBConnection || !m_DBConnection->Valid()){
        return;
    }

    auto pRecord = m_DBConnection->CreateDBRecord();

    if(pRecord->Execute("select fld_sid, fld_name from monster order by fld_sid")){
        if(pRecord->RowCount() > 0){
            while(pRecord->Fetch()){
                g_AddMonsterWindow->AddValid(pRecord->Get("fld_sid"), pRecord->Get("fld_name"));
            }
        }
    }
    m_DBConnection->DestroyDBRecord(pRecord);

    {
        std::lock_guard<std::mutex> stGuard(m_ValidMonsterTypeMutex);
        for(auto nCurrentType: m_ValidMonsterTypeV){
            auto pCurrentRecord = m_DBConnection->CreateDBRecord();
            char szCmd[512];
            std::sprintf(szCmd,
                    "select fld_sid, fld_name from monster where fld_sid = %d", nCurrentType);
            if(pCurrentRecord->Execute(szCmd)){
                if(pCurrentRecord->RowCount() > 0){
                    while(pCurrentRecord->Fetch()){
                        g_AddMonsterWindow->AddCurrent(pCurrentRecord->Get("fld_sid"), pCurrentRecord->Get("fld_name"));
                    }
                }
            }
            m_DBConnection->DestroyDBRecord(pCurrentRecord);
        }
    }

}

void SceneServer::AddMonsterType(uint32_t nType)
{
    std::lock_guard<std::mutex> stGuard(m_ValidMonsterTypeMutex);
    if(!std::binary_search(m_ValidMonsterTypeV.begin(), m_ValidMonsterTypeV.end(), nType)){
        m_ValidMonsterTypeV.push_back(nType);
    }
    std::sort(m_ValidMonsterTypeV.begin(), m_ValidMonsterTypeV.end());
}

bool SceneServer::TryAddMonster()
{
	extern ServerConfigureWindow *g_ServerConfigureWindow;
    if(m_MonsterList.size() > (int)(g_ServerConfigureWindow->MaxMonsterCount() * 0.5)){
        return false;
    }

    {
        std::lock_guard<std::mutex> stGuard(m_ValidMonsterTypeMutex);
        if(m_ValidMonsterTypeV.empty()){
            return false;
        }

        int nTypeIndex = std::rand() % m_ValidMonsterTypeV.size();
        auto pMonster = new Monster(
                (m_ValidMonsterTypeV[nTypeIndex] % 1024) + (1 << 10), m_MaxMonsterUID + 1, GetTimeMS());

        if(!RandomStartMonster(pMonster)){
            delete pMonster;
            return false;
        }else{
			pMonster->BroadcastBaseInfo();
            m_MonsterList.push_back(pMonster);
			m_MaxMonsterUID++;
            return true;
        }

        delete pMonster;
        return false;
    }

    return false;
}

bool SceneServer::PutMonsterOnMap(Monster *pMonster)
{
    int nX, nY, nTryCount;

    nTryCount = 0;
    while(nTryCount < 50){
        if(GetProperPosition(nX, nY, pMonster)){
            // MonsterList is locked already
			pMonster->SetMap(nX, nY, m_Map);
            for(auto pCurrentMonster: m_MonsterList){
                if(pMonster->Collide(pCurrentMonster)){
					return false;
                }
            }
            { // test human
                std::lock_guard<std::mutex> stGuard(m_HumanListMutex);
                for(auto pHuman: m_HumanList){
                    if(pHuman->Collide(pMonster)){
                        return false;
                    }
                }
            }
			return true;
        }
    }
    return false;
}

bool SceneServer::GetProperPosition(int &nX, int &nY, Monster *pMonster)
{
    // TODO add a table here to check for valid point
    // here we take consider of ground information
    int nTryCount = 0;
    while(nTryCount < 50){
        nX = std::rand() % (m_Map->Width()  * 48);
        nY = std::rand() % (m_Map->Height() * 32);
        if(m_Map->ValidPosition(nX, nY, pMonster)){
            return true;
        }
        nTryCount++;
    }
    return false;
}

void SceneServer::OnConnectSucceed()
{
    static Message stMessage;
    ServerMessageMapName stTmpSM;
    std::strcpy(stTmpSM.szMapName, m_MapName.c_str());
    stMessage.Set(SERVERMT_MAPNAME, stTmpSM);
    GetMessageManager()->SendMessage(stMessage);
}

bool SceneServer::RetrieveHumanCoverInfo()
{
    // load all rectcover info at one time
    // no sense to cut them
    if(!Actor::GlobalCoverInfoValid(3 << 10)){
        auto pRecord = m_DBConnection->CreateDBRecord();
        if(true
                && pRecord->Execute("select * from humancover")
                && pRecord->RowCount() > 0
          ){
            while(pRecord->Fetch()){
                Actor::SetGlobalCoverInfo(3 << 10,
                        std::atoi(pRecord->Get("fld_state")),
                        std::atoi(pRecord->Get("fld_direction")),
                        std::atoi(pRecord->Get("fld_w")),
                        std::atoi(pRecord->Get("fld_h")));
            }
        }else{
            return false;
        }
    }
    return true;
}

bool SceneServer::RetrieveMonsterCoverInfo(uint32_t nSID)
{
    // load all rectcover info at one time
    // no sense to cut them
    if(!Actor::GlobalCoverInfoValid(nSID)){
        auto pRecord = m_DBConnection->CreateDBRecord();
        char szCmd[512];
        std::sprintf(szCmd,
                "select * from monstercover where fld_sid = %d", nSID & 0X000003FF);
        if(pRecord->Execute(szCmd) && pRecord->RowCount() > 0){
            while(pRecord->Fetch()){
                Actor::SetGlobalCoverInfo(nSID,
                        std::atoi(pRecord->Get("fld_state")),
                        std::atoi(pRecord->Get("fld_direction")),
                        std::atoi(pRecord->Get("fld_w")),
                        std::atoi(pRecord->Get("fld_h")));
            }
        }else{
            return false;
        }
    }
    return true;
}

bool SceneServer::RandomStartMonster(Monster *pMonster)
{
    if(RetrieveMonsterCoverInfo(pMonster->SID())){
        if(pMonster->RandomStart() && PutMonsterOnMap(pMonster)){
            return true;
        }
    }
    return false;
}

void SceneServer::Broadcast(const Message &stMessage)
{
    // TODO simple implementation here
    std::lock_guard<std::mutex> stGuard(m_HumanListMutex);
    for(auto pHuman: m_HumanList){
        ServerMessageBroadcast stSMB;
        stSMB.nSID = pHuman->SID();
        stSMB.nUID = pHuman->UID();
        stSMB.nLen = stMessage.Size();
        // TODO alert when stMessage.Size() > sizeof(stSMB.Data)
        std::memcpy(stSMB.Data, stMessage.Data(), stMessage.Size());

        Message stTmpMSG;
        stTmpMSG.Set(SERVERMT_BROADCAST, stSMB);
        GetMessageManager()->SendMessage(stTmpMSG);
    }
}

void SceneServer::OnRecv(const Message &stMessage)
{
    extern MainWindow *g_MainWindow;
    // g_MainWindow->AddLog(0, "receive message from gate server");
    switch(stMessage.Index()){
        case SERVERMT_CONNECTSUCCEED:
            {
				g_MainWindow->AddLog(0, "connected to gate server successfully");
                break;
            }
        case SERVERMT_ADDHUMAN:
            {
                ServerMessageAddHuman stSMAH;
                std::memcpy(&stSMAH, stMessage.Body(), sizeof(stSMAH));
                DoAddHuman(stSMAH.nUID);
                break;
            }
        default:
            {
                extern MainWindow *g_MainWindow;
                std::string szInfo = "Unknown message ";
                szInfo += std::to_string(stMessage.Index());
                szInfo += " from gate server dropped";
                g_MainWindow->AddLog(1, szInfo.c_str());
                break;
            }
    }
}

void SceneServer::DoAddHuman(int nUID)
{
    if(!RetrieveHumanCoverInfo()){
        extern MainWindow *g_MainWindow;
        g_MainWindow->AddLog(2, "Retrieve human DRC info failed...");
        return;
    }

    auto pRecord = m_DBConnection->CreateDBRecord();
    char szQueryCmd[512];
    std::sprintf(szQueryCmd, "select * from userinfo where fld_uid = %d", nUID);

    if(pRecord->Execute(szQueryCmd)){
        if(pRecord->RowCount() != 0){
            pRecord->Fetch();
            {
                auto pHuman = new Human(
                        std::atoi(pRecord->Get("fld_sid")),
                        std::atoi(pRecord->Get("fld_uid")),
                        GetTimeMS());
                pHuman->SetDirection(
                        std::atoi(pRecord->Get("fld_direction")));
				// set map need to set DRC, which need direction
                pHuman->SetMap(
                        std::atoi(pRecord->Get("fld_x")),
                        std::atoi(pRecord->Get("fld_y")),
                        m_Map);
                pHuman->SetLevel(
                        std::atoi(pRecord->Get("fld_level")));
                std::lock_guard<std::mutex> stGuard(m_HumanListMutex);
                m_HumanList.push_back(pHuman);
				return;
            }
        }
    }
    extern MainWindow *g_MainWindow;
    g_MainWindow->AddLog(2, "Adding human failed...");

    Message stMessage;
    stMessage.SetIndex(SERVERMT_ADDHUMANERROR);
    GetMessageManager()->SendMessage(stMessage);
}

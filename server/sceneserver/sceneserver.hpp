#pragma once
#include <list>
#include <mutex>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstdint>
#include "database.hpp"
#include "message.hpp"
#include "monster.hpp"
#include "human.hpp"
#include "newmir2map.hpp"
#include "message.hpp"

class SceneServer final
{
    public:
        SceneServer();
        ~SceneServer();

    private:
        // TODO this also need to be protected by mutex
        DBConnection    *m_DBConnection;

    public:
        void Init();
        void Launch();

    private:
        bool LoadMap();

    private:
        std::chrono::time_point<std::chrono::system_clock> m_StartTime;
        uint32_t    m_UpdateTime;

    public:
        uint32_t    GetTimeMS();

    private:
        void        DelayMS(uint32_t);

    private:
        std::vector<uint32_t> m_ValidMonsterTypeV;
        std::mutex            m_ValidMonsterTypeMutex;

    public:
        void UpdateValidMonsterList();
        void AddMonsterType(uint32_t);
        void Broadcast(const Message &);

    private:
        bool StartServerLogicThread();
        bool ConnectToDatebase();
        bool ConnectToGateServer();

    public:
        void OnMessageRecv(const Message &);

    private:
        std::string     m_MapName;
    private:
        std::atomic<bool>   m_Quit;
        std::thread        *m_HumanThread;
        std::thread        *m_MonsterThread;

    private:
        std::list<Monster *>    m_MonsterList;
        std::mutex              m_MonsterListMutex;
        std::list<Human *>      m_HumanList;
        std::mutex              m_HumanListMutex;

    private:
        bool RequestQuit();

    private:
        void HumanThread();
        void MonsterThread();

        bool TryAddMonster();
        bool GetProperPosition(int &, int &, Monster *);
        bool PutMonsterOnMap(Monster *);

    private:
        bool RandomStartMonster(Monster *);
        bool RetrieveMonsterCoverInfo(uint32_t);
        bool RetrieveHumanCoverInfo();

    public:
        void OnConnectSucceed();
        void OnRecv(const Message &);

    public:
        void DoAddHuman(int);

    private:
        // int m_MaxMonsterCount;
        int m_MaxMonsterUID;

    private:
        NewMir2Map *m_Map;
};

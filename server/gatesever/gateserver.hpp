#pragma once
#include <unordered_map>
#include <mutex>
#include <list>

#include "session.hpp"
#include "database.hpp"
#include "sessionacceptor.hpp"

class GateServer final
{
    public:
        GateServer(int, int);
        ~GateServer();

    private:
        DBConnection    *m_UserInfoDB;

    private:
        std::mutex m_SceneServerMapMutex;
        std::unordered_map<int, std::string> m_SceneServerMap;

        std::mutex m_UIDLocationMutex;
        std::unordered_map<int, std::pair<int, int>> m_UIDLocation;

    private:
        SessionAcceptor m_ClientSessionAcceptor;
        SessionAcceptor m_SceneServerSessionAcceptor;

    public:
        void Init();
        void Launch();

    public:
        void OnClientRecv(const Message &, Session &);
        void OnServerRecv(const Message &, Session &);

    private:
        bool DoLogin(const Message &, Session &);
};

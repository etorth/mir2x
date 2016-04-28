/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
 *  Last Modified: 04/28/2016 00:22:00
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

#pragma once

#include <mutex>
#include <chrono>
#include <atomic>
#include <cstdint>
#include <type_traits>
#include <unordered_map>

#include "log.hpp"
#include "message.hpp"
#include "taskhub.hpp"
#include "database.hpp"
#include "servermap.hpp"
#include "sessionhub.hpp"
#include "eventtaskhub.hpp"


class ServiceCore;
class MonoServer: public SyncDriver
{
    protected:
        ServiceCore *m_ServiceCore;
        Theron::Address m_ServiceCoreAddress;

    private:
        typedef struct _NetMessageDesc{
            // attributes for 256 network messages
            size_t Size;
            bool   FixedSize;

            _NetMessageDesc()
                : Size(0)
                , FixedSize(true)
            {}
        }NetMessageDesc;

        std::array<NetMessageDesc, 256> m_NetMessageDescV;

    public:
        size_t MessageSize(uint8_t nMessageID)
        {
            return m_NetMessageDescV[nMessageID].Size;
        }

        bool MessageFixedSize(uint8_t nMessageID)
        {
            return m_NetMessageDescV[nMessageID].FixedSize;
        }

    public:
        MonoServer();
        ~MonoServer();

    public:
        void ReadHC();

        void Launch();
        void Restart();

    private:
        void RunASIO();
        void CreateDBConnection();

    private:
        void AddLog(int, const char *, ...);

    private:
        void ExtendLogBuf(size_t);

    private:
        // for network
        SessionHub   *m_SessionHub;

    private:
        std::atomic<uint32_t> m_ObjectUID;
    private:
        // for log
        size_t   m_LogBufSize;
        char    *m_LogBuf;

    private:
        // for DB
        DBConnection *m_DBConnection;

    private:
        Theron::Receiver             m_Receiver;
        Theron::Catcher<MessagePack> m_Catcher;

    public:
        Theron::Address GetAddress()
        {
            return m_Receiver.GetAddress();
        }

    private:
        bool AddPlayer(int, uint32_t);

    private:
        bool InitMonsterRace();
        bool InitMonsterItem();

    public:
        // for gui
        bool GetValidMapV(std::vector<std::pair<int, std::string>> &);
        bool GetValidMonsterV(int, std::vector<std::pair<int, std::string>> &);
        int  GetValidMonsterCount(int, int);

    public:
        uint32_t GetTickCount();

    protected:
        std::chrono::time_point<std::chrono::system_clock> m_StartTime;

    private:
        void OnReadHC(uint8_t, Session *);

        void OnWalk     (Session *);
        void OnPing     (Session *);
        void OnLogin    (Session *);
        void OnBroadcast(Session *);

    private:
        bool AddObject();

    public:
        // copy from class Log to support LOGTYPE_XXX
        template<typename... U> void AddLog(const std::array<std::string, 4> &stLoc, U&&... u)
        {
            extern Log *g_Log;
            g_Log->AddLog(stLoc, std::forward<U>(u)...);

            int nLevel = std::atoi(stLoc[0].c_str());
            AddLog(nLevel, std::forward<U>(u)...);
        }

    public:
        bool AddMonster(uint32_t, uint32_t, int, int, bool);
        bool AddMonster(uint32_t nMonsterInex, uint32_t nMapID, int nX, int nY)
        {
            return AddMonster(nMonsterInex, nMapID, nX, nY, false);
        }
};

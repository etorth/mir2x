/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
 *  Last Modified: 06/08/2016 23:16:43
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
#include "eventtaskhub.hpp"

class ServiceCore;
class MonoServer final
{
    private:
        // for log print and synchronization
        char       *m_LogBuf;
        size_t      m_LogBufSize;
        std::mutex  m_LogLock;

        std::mutex  m_DlgLock; // for fl_alert()
        std::atomic<uint32_t> m_ObjectUID;

    protected:
        ServiceCore *m_ServiceCore;
        Theron::Address m_SCAddress;

    private:
        typedef struct _NetMessageDesc{
            // attributes for 256 network messages
            uint8_t Type;
            size_t Size;
            bool   FixedSize;
            std::string Name;

            _NetMessageDesc(uint8_t nType = CM_UNKNOWN,
                    size_t nSize = 0,
                    bool bFixedSize = true,
                    const std::string &szName = "CM_UNKNOWN")
                : Type(nType)
                , Size(nSize)
                , FixedSize(bFixedSize)
                , Name(szName)
            {}
        }NetMessageDesc;

        // desc for *client* message
        // for server message we can directly get it's desc
        std::array<NetMessageDesc, 256> m_NetMessageDescV;

    public:
        const char *MessageName(uint8_t nMessageID)
        {
            return m_NetMessageDescV[nMessageID].Name.c_str();
        }

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
        void CreateServiceCore();
        void CreateDBConnection();

    private:
        void ExtendLogBuf(size_t);

    public:
        void AddLog(const std::array<std::string, 4> &, const char *, ...);

    private:
        // for DB
        DBConnection *m_DBConnection;

    private:
        bool AddPlayer(uint32_t, uint32_t);

    private:
        bool InitMonsterRace();
        bool InitMonsterItem();
        bool LoadMonsterRecord()
        {
           return InitMonsterRace() && InitMonsterItem();
        }

    private:
        void StartNetwork();

    public:
        // for gui
        bool GetValidMapV(std::vector<std::pair<int, std::string>> &);
        bool GetValidMonsterV(int, std::vector<std::pair<int, std::string>> &);
        int  GetValidMonsterCount(int, int);

    protected:
        std::chrono::time_point<std::chrono::system_clock> m_StartTime;

    public:
        void AddMonster(uint32_t, uint32_t, int, int);

    public:
        uint32_t GetUID()
        {
            return m_ObjectUID++;
        }

        uint32_t GetTimeTick()
        {
            // TODO
            // make it more simple
            return (uint32_t)std::chrono::duration_cast<
                std::chrono::milliseconds>(std::chrono::system_clock::now() - m_StartTime).count();
        }
};

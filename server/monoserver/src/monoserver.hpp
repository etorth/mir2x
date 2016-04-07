/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
 *  Last Modified: 04/06/2016 19:28:09
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

#include <cstdint>
#include "mapthread.hpp"
#include <unordered_map>
#include "sessionio.hpp"

#include "database.hpp"
#include "message.hpp"

#include "log.hpp"


class MonoServer final
{
    public:
        MonoServer();
        ~MonoServer();

    private:
        std::unordered_map<uint16_t, MapThread *>   m_MapPool;

    public:
        void Launch();
        void ReadHC();

    private:
        void RunASIO();
        void CreateDBConnection();

    private:
        void AddLog(int, const char *, ...);

    private:
        void ExtendLogBuf(size_t);

    private:
        // for network
        SessionIO   *m_SessionIO;

    private:
        // for log
        size_t   m_LogBufSize;
        char    *m_LogBuf;

    private:
        // for DB
        DBConnection *m_DBConnection;

    private:
        bool AddPlayer(int, uint32_t);

        std::mutex                        m_OnlinePlayerVLock;
        std::unordered_map<uint32_t, int> m_OnlinePlayerV;

    public:
        // for gui
        bool GetValidMapV(std::vector<std::pair<int, std::string>> &);
        bool GetValidMonsterV(int, std::vector<std::pair<int, std::string>> &);
        int  GetValidMonsterCount(int, int);

    private:
        void OnReadHC(uint8_t, Session *);

        void OnPing (Session *);
        void OnLogin(Session *);


    public:
        // copy from class Log to support LOGTYPE_XXX
        template<typename... U> void AddLog(const std::array<std::string, 4> &stLoc, U&&... u)
        {
            extern Log *g_Log;
            g_Log->AddLog(stLoc, std::forward<U>(u)...);

            int nLevel = std::atoi(stLoc[0].c_str());
            AddLog(nLevel, std::forward<U>(u)...);
        }
};

/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
 *  Last Modified: 03/01/2016 00:28:07
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
#include <cstdint>
#include "mapthread.hpp"
#include <unordered_map>
#include "sessionio.hpp"

#include "database.hpp"
#include "message.hpp"


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
        void Log(int, const char *, ...);

    private:
        void ExtendLogBuf(int);

    private:
        // for log
        char    *m_LogBuf;
        int      m_LogBufSize;

    private:
        // for network
        SessionIO   *m_SessionIO;

    private:
        // for DB
        DBConnection    *m_DBConnection;
        DBConnection    *m_UserInfoDB;

    private:
        bool PlayerLogin(SMLoginOK)
        {
            return true;
        }

    private:
        void OnReadHC(uint8_t, Session *);

        void OnPing (Session *);
        void OnLogin(Session *);
};

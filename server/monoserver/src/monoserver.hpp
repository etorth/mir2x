/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
 *  Last Modified: 02/29/2016 01:42:21
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


    private:
        void OnReadHC(uint8_t, Session *);
};

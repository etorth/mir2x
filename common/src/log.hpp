/*
 * =====================================================================================
 *
 *       Filename: log.hpp
 *        Created: 03/16/2016 16:05:17
 *  Last Modified: 03/16/2016 16:40:13
 *
 *    Description: log functionality enabled by g3Log
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
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <iomanip>
#include <thread>
#include <iostream>

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#define LOG_PATH "./"
#else
#define LOG_PATH "/tmp/"
#endif


#define MIR2X_VERSION_STRING "mir2x-v0.1"

#ifdef MIR2X_VERSION_STRING
#define LOG_ARGV0 MIR2X_VERSION_STRING
#else
#define LOG_ARGV0 "mir2x"
#endif

class Log final
{
    private:
        std::unique_ptr<FileSinkHandle> m_Handler;
        std::unique_ptr<LogWorker>      m_Worker;

    public:
        Log()
        {
            extern Log *g_Log;
            if(g_Log){ throw std::runtime_error("only one Log instance please."); }

            m_Worker  = g3::LogWorker::createLogWorker();
            m_Handler = m_Worker->addDefaultLogger(LOG_ARGV0, LOG_PATH);
            g3::initializeLogging(m_Worker.get());

            std::future<std::string> szLogFileName = m_Handler->call(&g3::FileSink::fileName);

            std::cout << "* This is the initialization of Log functionality"           << std::endl;
            std::cout << "* For info/debug/warning/fatal messages."                    << std::endl;
            std::cout << "* Log file: [" << szLogFileName.get() << "]"                 << std::endl;
            std::cout << "* Log functionality established!"                            << std::endl;
            std::cout << "* All messges will be redirected to the log after this line" << std::endl;
        }

        ~Log() = default;

    public:

        void Log(int nLevel, const char * szFormat, ...)
        {
        }
};

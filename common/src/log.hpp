/*
 * =====================================================================================
 *
 *       Filename: log.hpp
 *        Created: 03/16/2016 16:05:17
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
#include <array>
#include <thread>
#include <string>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

#include "logtype.hpp"

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
    public:
        enum {
            LOGTYPEV_INFO    = 0,
            LOGTYPEV_WARNING = 1,
            LOGTYPEV_FATAL   = 2,
            LOGTYPEV_DEBUG   = 3,
        };

    private:
        std::unique_ptr<g3::LogWorker>      m_Worker;
        std::unique_ptr<g3::FileSinkHandle> m_Handler;
        std::string                         m_LogFileName;

    public:
        Log(const char *szLogArg0 = LOG_ARGV0, const char *szLogPath = LOG_PATH)
            : m_Worker(g3::LogWorker::createLogWorker())
            , m_Handler(m_Worker->addDefaultLogger(szLogArg0, szLogPath))
        {
            g3::initializeLogging(m_Worker.get());

            std::future<std::string> szLogFileName = m_Handler->call(&g3::FileSink::fileName);

            std::cout << "* This is the initialization of Log functionality"           << std::endl;
            std::cout << "* For info/debug/warning/fatal messages."                    << std::endl;

            m_LogFileName = szLogFileName.get();

            std::cout << "* Log file: [" << m_LogFileName << "]"                       << std::endl;
            std::cout << "* Log functionality established!"                            << std::endl;
            std::cout << "* All messges will be redirected to the log after this line" << std::endl;
        }

        ~Log() = default;

    public:
        const char *LogPath() const
        {
            return m_LogFileName.c_str();
        }

    private:
        decltype(INFO) GetLevel(const char *szLevel)
        {
            if(!std::strcmp(szLevel, "0")){ return INFO;    }
            if(!std::strcmp(szLevel, "1")){ return WARNING; }
            if(!std::strcmp(szLevel, "2")){ return FATAL;   }

            return DEBUG;
        }

    public:
        void AddLog(const std::array<std::string, 4> &stLoc, const char *szInfo)
        {
            int nLine = std::atoi(stLoc[2].c_str());
            auto stLevel = GetLevel(stLoc[0].c_str());
            LogCapture(stLoc[1].c_str(), nLine, stLoc[3].c_str(), stLevel).capturef("%s", szInfo);
        }

        template<typename... U> void AddLog(const std::array<std::string, 4> &stLoc, const char *szLogFormat, U&&... u)
        {
            int nLine = std::atoi(stLoc[2].c_str());
            auto stLevel = GetLevel(stLoc[0].c_str());
            LogCapture(stLoc[1].c_str(), nLine, stLoc[3].c_str(), stLevel).capturef(szLogFormat, std::forward<U>(u)...);
        }
};

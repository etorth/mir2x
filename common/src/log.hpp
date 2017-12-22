/*
 * =====================================================================================
 *
 *       Filename: log.hpp
 *        Created: 03/16/2016 16:05:17
 *  Last Modified: 12/20/2017 01:36:07
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
#include <thread>
#include <string>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <array>

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


#define LOGTYPE_INFO    {std::string("0"), std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}
#define LOGTYPE_WARNING {std::string("1"), std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}
#define LOGTYPE_FATAL   {std::string("2"), std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}
#define LOGTYPE_DEBUG   {std::string("3"), std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}

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

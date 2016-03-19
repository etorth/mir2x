/*
 * =====================================================================================
 *
 *       Filename: log.hpp
 *        Created: 03/16/2016 16:05:17
 *  Last Modified: 03/18/2016 19:12:04
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
#include <string>

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


#define LOGTYPE_DEBUG   {std::string("-1"),\
    std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}
#define LOGTYPE_INFO    {std::string("0" ), \
    std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}
#define LOGTYPE_WARNING {std::string("1" ), \
    std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}
#define LOGTYPE_FATAL   {std::string("1" ), \
    std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}

class Log final
{
    private:
        std::unique_ptr<g3::FileSinkHandle> m_Handler;
        std::unique_ptr<g3::LogWorker>      m_Worker;
        std::string                         m_LogFileName;

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

            m_LogFileName = szLogFileName.get();

            std::cout << "* Log file: [" << m_LogFileName << "]"                       << std::endl;
            std::cout << "* Log functionality established!"                            << std::endl;
            std::cout << "* All messges will be redirected to the log after this line" << std::endl;
        }

        ~Log() = default;

    public:

        const char *FileName() const
        {
            return m_LogFileName.c_str();
        }

    public:

        template<typename... U> void AddLog(
                const std::array<std::string, 4> & stLoc, // use defined to subsistute __LINE__ etc.
                const char *szLogFormat, U&&... u)        // varidic for internal printf(...)
        {
            int nLine = std::atoi(stLoc[2].c_str());

            auto stLevel = INFO;
            if(stLoc[0] == "0"){
                stLevel = INFO;
            }else if(stLoc[0] == "1"){
                stLevel = WARNING;
            }else if(stLoc[0] == "2"){
                stLevel = FATAL;
            }else{
                stLevel = DEBUG;
            }

            LogCapture(stLoc[1].c_str(),
                    nLine, stLoc[3].c_str(), stLevel).capturef(szLogFormat, std::forward<U>(u)...);
        }
};

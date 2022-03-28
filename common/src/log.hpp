#pragma once
#include <tuple>
#include <string>
#include <cstring>
#include <iostream>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

#include "strf.hpp"
#include "logprof.hpp"
#include "fflerror.hpp"

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
        enum LogType: int
        {
            LOGTYPEV_INFO    = 0,
            LOGTYPEV_WARNING = 1,
            LOGTYPEV_FATAL   = 2,
            LOGTYPEV_DEBUG   = 3,
        };

    public:
        using LogTypeLoc = std::tuple<LogType, const char *, int, const char *>;

    private:
        std::unique_ptr<g3::LogWorker>      m_worker;
        std::unique_ptr<g3::FileSinkHandle> m_handler;

    private:
        std::string m_logFileName;

    public:
        Log(const char *logArg0 = LOG_ARGV0, const char *logPath = LOG_PATH)
            : m_worker(g3::LogWorker::createLogWorker())
            , m_handler(m_worker->addDefaultLogger(logArg0, logPath))
        {
            g3::initializeLogging(m_worker.get());
            std::future<std::string> logFileNameFeature = m_handler->call(&g3::FileSink::fileName);

            std::cout << "* This is the initialization of Log functionality"           << std::endl;
            std::cout << "* For info/debug/warning/fatal messages."                    << std::endl;

            m_logFileName = logFileNameFeature.get();

            std::cout << "* Log file: [" << m_logFileName << "]"                       << std::endl;
            std::cout << "* Log functionality established!"                            << std::endl;
            std::cout << "* All messges will be redirected to the log after this line" << std::endl;
        }

    public:
        ~Log() = default;

    public:
        const char *logFileName() const
        {
            return m_logFileName.c_str();
        }

    private:
        static decltype(INFO) getLevel(int type)
        {
            switch(type){
                case LOGTYPEV_INFO   : return INFO;
                case LOGTYPEV_WARNING: return WARNING;
                case LOGTYPEV_FATAL  : return FATAL;
                case LOGTYPEV_DEBUG  : return DEBUG;
                default              : throw fflerror("invalid log type: %d", type);
            }
        }

    public:
        void addLog(const LogTypeLoc &typeLoc, const char *format, ...)
        {
            std::string logLine;
            str_format(format, logLine);
            LogCapture(std::get<1>(typeLoc), std::get<2>(typeLoc), std::get<3>(typeLoc), getLevel(std::get<0>(typeLoc))).capturef("%s", logLine.c_str());
        }
};

#define LOGTYPE_INFO    {Log::LOGTYPEV_INFO   , __FILE__, __LINE__, __PRETTY_FUNCTION__}
#define LOGTYPE_WARNING {Log::LOGTYPEV_WARNING, __FILE__, __LINE__, __PRETTY_FUNCTION__}
#define LOGTYPE_FATAL   {Log::LOGTYPEV_FATAL  , __FILE__, __LINE__, __PRETTY_FUNCTION__}
#define LOGTYPE_DEBUG   {Log::LOGTYPEV_DEBUG  , __FILE__, __LINE__, __PRETTY_FUNCTION__}

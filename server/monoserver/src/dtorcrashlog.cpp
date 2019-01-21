/*
 * =====================================================================================
 *
 *       Filename: dtorcrashlog.cpp
 *        Created: 01/21/2019 21:37:10
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

#include "strfunc.hpp"
#include "monoserver.hpp"
#include "dtorcrashlog.hpp"

void DtorCrashLog(const char *szFormat, ...)
{
    va_list ap;
    va_start(ap, szFormat);

    std::string szLog;
    try{
        szLog = str_vprintf(szFormat, ap);
        va_end(ap);
        return szLog;
    }catch(...){
        szLog = (std::string("exception caught in str_vprintf(\"") + (szFormat ? szFormat : "(nullptr)")) + "\")";
        va_end(ap);
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_FATAL, "%s", szLog.c_str());
    std::abort();
}

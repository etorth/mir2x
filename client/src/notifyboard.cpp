/*
 * =====================================================================================
 *
 *       Filename: notifyboard.cpp
 *        Created: 02/13/2018 19:29:05
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

#include "log.hpp"
#include "global.hpp"
#include "condcheck.hpp"
#include "sdldevice.hpp"
#include "notifyboard.hpp"

void NotifyBoard::Pop()
{
    // remove the first log line
    if(!m_LogQueue.empty()){
        auto stHead = m_LogQueue.front();
        if(stHead.LineCount > 0){
            m_LogBoard.RemoveLine(0, stHead.LineCount);
        }
        m_LogQueue.pop();
    }
}

void NotifyBoard::Update(double)
{
    int nLineCount = 0;
    auto nCurrTick = (uint32_t)(SDL_GetTicks());
    while(!m_LogQueue.empty()){
        if(m_LogQueue.front().ExpireTime >= nCurrTick){
            nLineCount += (int)(m_LogQueue.front().LineCount);
            m_LogQueue.pop();
        }
    }

    if(nLineCount){
        m_LogBoard.RemoveLine(0, nLineCount);
    }
}

void NotifyBoard::AddXML(const char *szXML, const std::map<std::string, std::function<void()>> &rstMap)
{
    bool bRes = true;
    if(!m_LogBoard.Empty(false)){
        bRes = m_LogBoard.AppendXML("<ROOT><OBJECT TYPE=\"RETURN\"></OBJECT></ROOT>", {});
    }

    auto nLineCount0 = m_LogBoard.GetLineCount();
    if(bRes){
        bRes = m_LogBoard.AppendXML(szXML, rstMap);
    }

    auto nLineCount1 = m_LogBoard.GetLineCount();
    condcheck(nLineCount1 > nLineCount0);

    if(bRes){
        m_LogQueue.push({(uint32_t)(nLineCount1 - nLineCount0), (uint32_t)(SDL_GetTicks()) + 5000});
    }
}

void NotifyBoard::AddLog(std::array<std::string, 4> stLogType, const char *szLogFormat, ...)
{
    std::string szLog;
    bool bError = false;
    {
        va_list ap;
        va_start(ap, szLogFormat);

        try{
            szLog = str_vprintf(szLogFormat, ap);
        }catch(const std::exception &e){
            bError = true;
            szLog = str_printf("Exception caught in NotifyBoard::AddLog(\"%s\", ...): %s", szLogFormat, e.what());
        }

        va_end(ap);
    }

    int nLogType = bError ? Log::LOGTYPEV_WARNING : std::atoi(stLogType[0].c_str());
    switch(nLogType){
        case Log::LOGTYPEV_INFO:
            {
                AddXML(str_printf("<ROOT><OBJECT TYPE=\"PLAINTEXT\" COLOR=\"WHITE\">%s</OBJECT></ROOT>", szLog.c_str()).c_str(), {});
                return;
            }
        case Log::LOGTYPEV_WARNING:
            {
                AddXML(str_printf("<ROOT><OBJECT TYPE=\"PLAINTEXT\" COLOR=\"BROWN\">%s</OBJECT></ROOT>", szLog.c_str()).c_str(), {});
                return;
            }
        case Log::LOGTYPEV_FATAL:
            {
                AddXML(str_printf("<ROOT><OBJECT TYPE=\"PLAINTEXT\" COLOR=\"RED\">%s</OBJECT></ROOT>", szLog.c_str()).c_str(), {});
                return;
            }
        case Log::LOGTYPEV_DEBUG:
            {
                AddXML(str_printf("<ROOT><OBJECT TYPE=\"PLAINTEXT\" COLOR=\"GREEN\">%s</OBJECT></ROOT>", szLog.c_str()).c_str(), {});
                return;
            }
        default:
            {
                g_Log->AddLog(LOGTYPE_WARNING, "Invalid LogType %d: %s", nLogType, szLog.c_str());
                AddXML(str_printf("<ROOT><OBJECT TYPE=\"PLAINTEXT\" COLOR=\"RED\">Invalid LogType %d: %s</OBJECT></ROOT>", nLogType, szLog.c_str()).c_str(), {});
                return;
            }
    }
}

void NotifyBoard::DrawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
{
    m_LogBoard.DrawEx(nDstX, nDstY, nSrcX, nSrcY, nW, nH);
}

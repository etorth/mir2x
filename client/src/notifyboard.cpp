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
#include "logtype.hpp"
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
    auto fnRecordError = [](const char *szError)
    {
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_WARNING, "%s", szError);
    };


    auto fnRecordLog = [this, fnRecordError](int nLogType, const char *szLogInfo)
    {
        switch(nLogType){
            case LOGV_INFO:
                {
                    std::string szXMLContent;

                    szXMLContent += "<ROOT><OBJECT TYPE=\"PLAINTEXT\" COLOR=\"WHITE\">";
                    szXMLContent += szLogInfo ? szLogInfo : "";
                    szXMLContent += "</OBJECT></ROOT>";

                    AddXML(szXMLContent.c_str(), {});
                    return;
                }
            case LOGV_WARNING:
                {
                    std::string szXMLContent;

                    szXMLContent += "<ROOT><OBJECT TYPE=\"PLAINTEXT\" COLOR=\"BROWN\">";
                    szXMLContent += szLogInfo ? szLogInfo : "";
                    szXMLContent += "</OBJECT></ROOT>";

                    AddXML(szXMLContent.c_str(), {});
                    return;
                }
            case LOGV_FATAL:
                {
                    std::string szXMLContent;

                    szXMLContent += "<ROOT><OBJECT TYPE=\"PLAINTEXT\" COLOR=\"RED\">";
                    szXMLContent += szLogInfo ? szLogInfo : "";
                    szXMLContent += "</OBJECT></ROOT>";

                    AddXML(szXMLContent.c_str(), {});
                    return;
                }
            case LOGV_DEBUG:
                {
                    std::string szXMLContent;

                    szXMLContent += "<ROOT><OBJECT TYPE=\"PLAINTEXT\" COLOR=\"GREEN\">";
                    szXMLContent += szLogInfo ? szLogInfo : "";
                    szXMLContent += "</OBJECT></ROOT>";

                    AddXML(szXMLContent.c_str(), {});
                    return;
                }
            default:
                {
                    std::string szXMLContent;

                    szXMLContent += "<ROOT><OBJECT TYPE=\"PLAINTEXT\" COLOR=\"RED\">";
                    szXMLContent += "Invalid LogType: ";
                    szXMLContent += std::to_string(nLogType);
                    szXMLContent += " : ";
                    szXMLContent += szLogInfo ? szLogInfo : "";
                    szXMLContent += "</OBJECT></ROOT>";

                    AddXML(szXMLContent.c_str(), {});

                    std::string szErrorInfo;
                    szErrorInfo += "Invalid LogType: ";
                    szErrorInfo += std::to_string(nLogType);
                    szErrorInfo += " : ";
                    szErrorInfo += szLogInfo;

                    fnRecordError(szErrorInfo.c_str());
                    return;
                }
        }
    };

    va_list ap;
    va_start(ap, szLogFormat);
    LogStr::AddLog(stLogType, szLogFormat, ap, fnRecordLog, fnRecordError);
    va_end(ap);
}

void NotifyBoard::DrawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
{
    m_LogBoard.DrawEx(nDstX, nDstY, nSrcX, nSrcY, nW, nH);
}

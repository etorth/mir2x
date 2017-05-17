/*
 * =====================================================================================
 *
 *       Filename: noticenode.cpp
 *        Created: 05/14/2017 10:39:17
 *  Last Modified: 05/14/2017 10:41:05
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

#include <cstdint>
#include <cinttypes>

#include "log.hpp"
#include "noticenode.hpp"

void NoticeNode::Print() const
{
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "NoticeNode::this        = 0X%0*X", (int)(sizeof(this) * 2), (uintptr_t)(this));
    g_Log->AddLog(LOGTYPE_INFO, "          ::Notice      = %d", Notice);
    g_Log->AddLog(LOGTYPE_INFO, "          ::NoticeParam = %d", NoticeParam);
    g_Log->AddLog(LOGTYPE_INFO, "          ::X           = %d", X);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Y           = %d", Y);
}

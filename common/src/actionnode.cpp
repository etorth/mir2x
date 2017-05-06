/*
 * =====================================================================================
 *
 *       Filename: actionnode.cpp
 *        Created: 04/07/2017 12:46:54
 *  Last Modified: 05/05/2017 17:49:55
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
#include "actionnode.hpp"

void ActionNode::Print() const
{
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "ActionNode::this        = 0X%0*X", (int)(sizeof(this) * 2), (uintptr_t)(this));
    g_Log->AddLog(LOGTYPE_INFO, "          ::Action      = %d", Action);
    g_Log->AddLog(LOGTYPE_INFO, "          ::ActionParam = %d", ActionParam);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Direction   = %d", Direction);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Speed       = %d", Speed);
    g_Log->AddLog(LOGTYPE_INFO, "          ::X           = %d", X);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Y           = %d", Y);
    g_Log->AddLog(LOGTYPE_INFO, "          ::EndX        = %d", EndX);
    g_Log->AddLog(LOGTYPE_INFO, "          ::EndY        = %d", EndY);
    g_Log->AddLog(LOGTYPE_INFO, "          ::MapID       = %d", (int)(MapID));
}

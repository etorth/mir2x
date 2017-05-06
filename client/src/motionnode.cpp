/*
 * =====================================================================================
 *
 *       Filename: motionnode.cpp
 *        Created: 04/05/2017 12:40:09
 *  Last Modified: 05/05/2017 17:53:37
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
#include <cinttypes>
#include "log.hpp"
#include "motionnode.hpp"

void MotionNode::Print()
{
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::this      = 0X%0*X", (int)(2 * sizeof(this)), (uintptr_t)(this));
    g_Log->AddLog(LOGTYPE_INFO, "          ::Motion    = %d", Motion);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Direction = %d", Direction);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Speed     = %d", Speed);
    g_Log->AddLog(LOGTYPE_INFO, "          ::X         = %d", X);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Y         = %d", Y);
    g_Log->AddLog(LOGTYPE_INFO, "          ::EndX      = %d", EndX);
    g_Log->AddLog(LOGTYPE_INFO, "          ::EndY      = %d", EndY);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Frame     = %d", Frame);
}

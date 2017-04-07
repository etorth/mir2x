/*
 * =====================================================================================
 *
 *       Filename: motionnode.cpp
 *        Created: 04/05/2017 12:40:09
 *  Last Modified: 04/07/2017 13:27:05
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
#include "log.hpp"
#include "motionnode.hpp"

void MotionNode::Print()
{
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::this      = 0X%0*X", sizeof(this), this);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Motion    = %d", Motion);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Direction = %d", Direction);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Speed     = %d", Speed);
    g_Log->AddLog(LOGTYPE_INFO, "          ::X         = %d", X);
    g_Log->AddLog(LOGTYPE_INFO, "          ::Y         = %d", Y);
    g_Log->AddLog(LOGTYPE_INFO, "          ::EndX      = %d", EndX);
    g_Log->AddLog(LOGTYPE_INFO, "          ::EndY      = %d", EndY);
}

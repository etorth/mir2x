/*
 * =====================================================================================
 *
 *       Filename: motionnode.cpp
 *        Created: 04/05/2017 12:40:09
 *  Last Modified: 08/05/2017 22:56:47
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
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::Motion      = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Motion     );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::MotionParam = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), MotionParam);
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::Direction   = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Direction  );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::Speed       = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Speed      );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::X           = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), X          );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::Y           = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Y          );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::EndX        = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), EndX       );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::EndY        = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), EndY       );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::Frame       = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Frame      );
}

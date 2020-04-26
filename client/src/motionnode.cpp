/*
 * =====================================================================================
 *
 *       Filename: motionnode.cpp
 *        Created: 04/05/2017 12:40:09
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

extern Log *g_Log;

void MotionNode::print() const
{
    g_Log->addLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::motion      = %s", (int)(2 * sizeof(this)), (uintptr_t)(this), MotionNode::name(motion));
    g_Log->addLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::motionParam = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), motionParam            );
    g_Log->addLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::direction   = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), direction              );
    g_Log->addLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::speed       = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), speed                  );
    g_Log->addLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::x           = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), x                      );
    g_Log->addLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::y           = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), y                      );
    g_Log->addLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::endX        = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), endX                   );
    g_Log->addLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::endY        = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), endY                   );
    g_Log->addLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::frame       = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), frame                  );
    g_Log->addLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::fadeOut     = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), fadeOut                );
}

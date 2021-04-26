/*
 * =====================================================================================
 *
 *       Filename: guard.cpp
 *        Created: 04/26/2021 02:32:45
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

#include "guard.hpp"

Guard::Guard(uint32_t monID, ServiceCore *corePtr, ServerMap *mapPtr, int argX, int argY, int argDir)
    : Monster(monID, corePtr, mapPtr, argX, argY, argDir, 0)
{}

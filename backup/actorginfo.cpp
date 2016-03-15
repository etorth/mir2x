/*
 * =====================================================================================
 *
 *       Filename: actorginfo.cpp
 *        Created: 03/12/2016 01:30:18
 *  Last Modified: 03/12/2016 18:45:57
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

#include "actorginfo.hpp"

ActorGInfo::ActorGInfo()
{
    std::memset(m_FrameCount, 0, 32 * 8 * sizeof(int));
}

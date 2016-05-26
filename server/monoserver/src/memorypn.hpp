/*
 * =====================================================================================
 *
 *       Filename: memorypn.hpp
 *        Created: 05/24/2016 19:11:41
 *  Last Modified: 05/26/2016 15:16:23
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

#pragma once
#include "memorychunkpn.hpp"

class MemoryPN: public MemoryChunkPN<64, 256, 4>
{
    public:
        MemoryPN();
        ~MemoryPN() = default;
};

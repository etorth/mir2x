/*
 * =====================================================================================
 *
 *       Filename: memorypn.hpp
 *        Created: 05/24/2016 19:11:41
 *  Last Modified: 05/24/2016 19:17:17
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
#include "memoryblockpn.hpp"

class MemoryPN: public MemoryBlockPN<64, 1024, 4>
{
    public:
        MemoryPN();
        ~MemoryPN() = default;
};

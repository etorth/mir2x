/*
 * =====================================================================================
 *
 *       Filename: memoryblockpn.hpp
 *        Created: 05/12/2016 23:00:52
 *  Last Modified: 05/16/2016 17:46:23
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
#include "memoryblockpool.hpp"

template<size_t BlockSize>
using MemoryBlockPN = MemoryBlockPool<BlockSize, 1024>;


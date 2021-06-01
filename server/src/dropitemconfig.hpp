/*
 * =====================================================================================
 *
 *       Filename: dropitemconfig.hpp
 *        Created: 07/30/2017 00:11:10
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
#include <map>
#include <vector>
#include <cstdint>
#include "serdesmsg.hpp"

struct DropItemConfig
{
    uint32_t itemID = 0;
    int probRecip   = 0;
    int count       = 0;
};

std::vector<SDItem> getMonsterDropItemList(uint32_t);
const std::map<int, std::vector<DropItemConfig>> &getMonsterDropItemConfigList(uint32_t);

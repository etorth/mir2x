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

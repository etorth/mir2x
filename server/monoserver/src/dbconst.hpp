/*
 * =====================================================================================
 *
 *       Filename: dbconst.hpp
 *        Created: 05/12/2017 17:57:04
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
#include "dcrecord.hpp"
#include "dropitem.hpp"
#include "monsterrecord.hpp"

const DCRecord &DB_DCRECORD(uint32_t);

const std::map<int, std::vector<DropItem>> &DB_MONSTERDROPITEM(uint32_t);
const std::map<int, std::vector<DropItem>> &DB_MONSTERDROPITEM(const char *);

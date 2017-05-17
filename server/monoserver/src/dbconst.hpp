/*
 * =====================================================================================
 *
 *       Filename: dbconst.hpp
 *        Created: 05/12/2017 17:57:04
 *  Last Modified: 05/14/2017 09:38:09
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
#include <cstdint>
#include "dcrecord.hpp"
#include "monsterrecord.hpp"

const DCRecord      &DB_DCRECORD(uint32_t);
const MonsterRecord &DB_MONSTERRECORD(uint32_t);

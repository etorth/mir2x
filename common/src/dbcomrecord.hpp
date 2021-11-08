/*
 * =====================================================================================
 *
 *       Filename: dbcomrecord.hpp
 *        Created: 07/28/2017 23:03:43
 *    Description: split from dbcom.hpp
 *                 don't include dbcomid.hpp in this header file
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
#include "maprecord.hpp"
#include "buffrecord.hpp"
#include "itemrecord.hpp"
#include "magicrecord.hpp"
#include "monsterrecord.hpp"

const ItemRecord &DBCOM_ITEMRECORD(uint32_t);
const ItemRecord &DBCOM_ITEMRECORD(const char8_t *);

const MagicRecord &DBCOM_MAGICRECORD(uint32_t);
const MagicRecord &DBCOM_MAGICRECORD(const char8_t *);

const MonsterRecord &DBCOM_MONSTERRECORD(uint32_t);
const MonsterRecord &DBCOM_MONSTERRECORD(const char8_t *);

const MapRecord &DBCOM_MAPRECORD(uint32_t);
const MapRecord &DBCOM_MAPRECORD(const char8_t *);

const BuffRecord &DBCOM_BUFFRECORD(uint32_t);
const BuffRecord &DBCOM_BUFFRECORD(const char8_t *);

const BuffTriggerRecord &DBCOM_BUFFTRIGGERRECORD(uint32_t);
const BuffTriggerRecord &DBCOM_BUFFTRIGGERRECORD(const char8_t *);

bool getClothGender(uint32_t);

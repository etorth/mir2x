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

const BuffActRecord &DBCOM_BUFFACTRECORD(uint32_t);
const BuffActRecord &DBCOM_BUFFACTRECORD(const char8_t *);

const AttackModifierRecord &DBCOM_ATTACKMODIFIERRECORD(uint32_t);
const AttackModifierRecord &DBCOM_ATTACKMODIFIERRECORD(const char8_t *);

const SpellModifierRecord &DBCOM_SPELLMODIFIERRECORD(uint32_t);
const SpellModifierRecord &DBCOM_SPELLMODIFIERRECORD(const char8_t *);

bool getClothGender(uint32_t);

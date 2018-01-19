/*
 * =====================================================================================
 *
 *       Filename: invardata.hpp
 *        Created: 09/23/2017 13:04:02
 *  Last Modified: 01/18/2018 23:03:25
 *
 *    Description: every actor has invariant attributes
 *                 for the scenario A want to know B::m_ConstAttrib
 *
 *                 1. A query B::m_ConstAttrib every time
 *                 2. A query B::m_ConstAttrib for one time and keep a record
 *                 3. A query B to get UIDRecord::Invar::m_ConstAttrib
 *
 *                 method-3 is the most simple way
 *                 but UIDRecord won't take care of specific class information
 *
 *                 provide:
 *                      InvarData UIDRecord::Desp
 *
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
#include <cstring>
#include <cinttypes>

union InvarData
{
    struct _MonsterDesp
    {
        uint32_t MonsterID;
    }Monster;

    struct _PlayerDesp
    {
        uint32_t DBID;
    }Player;

    struct _MiscDesp
    {
        uint8_t Data[32];
    }Misc;

    InvarData()
    {
        std::memset(this, 0, sizeof(*this));
    }
};

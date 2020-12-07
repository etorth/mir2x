/*
 * =====================================================================================
 *
 *       Filename: dropitem.hpp
 *        Created: 07/28/2017 12:25:39
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

struct DropItem
{
    uint32_t ID;

    int ProbRecip;
    int Value;

    DropItem(uint32_t nID, int nProbRecip, int nValue)
        : ID(nID)
        , ProbRecip(nProbRecip)
        , Value(nValue)
    {}
};

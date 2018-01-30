/*
 * =====================================================================================
 *
 *       Filename: dbhuman.hpp
 *        Created: 05/26/2017 19:13:59
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

struct HumanParam
{
    int Job;
    int Level;
    int HPMax;
    int MPMax;

    HumanParam(
            int nJob,
            int nLevel,
            int nHPMax,
            int nMPMax)
        : Job(nJob)
        , Level(nLevel)
        , HPMax(nHPMax)
        , MPMax(nMPMax)
    {}
};

const HumanParam &DB_HUMANPARAM(int, int);

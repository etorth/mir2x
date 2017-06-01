/*
 * =====================================================================================
 *
 *       Filename: dbhuman.cpp
 *        Created: 05/26/2017 19:18:29
 *  Last Modified: 05/27/2017 00:45:10
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
#include <unordered_map>
#include "dbhuman.hpp"
const HumanParam &DB_HUMANPARAM(int nJobID, int nLevel)
{
    static const std::unordered_map<uint32_t, HumanParam> stHumanParamRecord
    {
        {((0 << 16) | (0)), {0, 0,  0,  0}},
        {((1 << 16) | (1)), {1, 1, 10, 10}},
    };

    uint32_t nKey = ((uint32_t)(nJobID & 0XFFFF) << 16) + (uint32_t)(nLevel & 0XFFFF);
    return (stHumanParamRecord.find(nKey) == stHumanParamRecord.end()) ? stHumanParamRecord.at(0) : stHumanParamRecord.at(nKey);
}

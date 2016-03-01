/*
 * =====================================================================================
 *
 *       Filename: guiapi.cpp
 *        Created: 03/01/2016 01:07:45
 *  Last Modified: 03/01/2016 01:39:29
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

#include <vector>
#include <utility>
#include "monoserver.hpp"

bool MonoServer::GetValidMapV(std::vector<std::pair<int, std::string>> &stMapV)
{
    stMapV.clear();
    return true;
}

bool MonoServer::GetValidMonsterV(int nMapID, std::vector<std::pair<int, std::string>> &stMonsterV)
{
    nMapID /= 2;
    stMonsterV.clear();
    return true;
}

int MonoServer::GetValidMonsterCount(int nMapID, int nMonsterID)
{
    return nMapID + nMonsterID;
}

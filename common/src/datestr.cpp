/*
 * =====================================================================================
 *
 *       Filename: datestr.cpp
 *        Created: 08/21/2017 16:07:19
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

#include <ctime>
#include "datestr.hpp"

std::string DateStr::Now()
{
    std::time_t stRawTime;
    std::string szFormatTime;

    if(std::time(&stRawTime) != (std::time_t)(-1)){
        // not thread-safe here
        // localtime() return an internal std::tm buffer
        if(auto pLocal = std::localtime(&stRawTime)){
            char szTimeBuffer[128];
            std::strftime(szTimeBuffer, sizeof(szTimeBuffer), "%Y%m%d%H%M%S", pLocal);
            szFormatTime = szTimeBuffer;
        }
    }
    return szFormatTime;
}

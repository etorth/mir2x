/*
 * =====================================================================================
 *
 *       Filename: serverenv.hpp
 *        Created: 05/12/2017 16:33:25
 *  Last Modified: 12/09/2017 13:21:10
 *
 *    Description: use environment to setup the runtime message report:
 *
 *                 MIR2X_INFO_XXXX      : configured one by one
 *                 MIR2X_CONFIG_XXXX    : configured one by one
 *
 *                 MIR2X_DEBUG_XXXX     : enable automatically if MIR2X_DEBUG level OK
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
#include <string>
#include <cstdint>

struct ServerEnv
{
    int  MIR2X_DEBUG;
    bool MIR2X_DEBUG_PRINT_AM_COUNT;
    bool MIR2X_DEBUG_PRINT_AM_FORWARD;

    ServerEnv()
    {
        MIR2X_DEBUG = std::getenv("MIR2X_DEBUG") ? std::atoi(std::getenv("MIR2X_DEBUG")) : 0;

        MIR2X_DEBUG_PRINT_AM_COUNT   = (MIR2X_DEBUG >= 5) ? true : (std::getenv("MIR2X_DEBUG_PRINT_AM_COUNT"  ) ? true : false);
        MIR2X_DEBUG_PRINT_AM_FORWARD = (MIR2X_DEBUG >= 5) ? true : (std::getenv("MIR2X_DEBUG_PRINT_AM_FORWARD") ? true : false);
    }
};

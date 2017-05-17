/*
 * =====================================================================================
 *
 *       Filename: serverenv.hpp
 *        Created: 05/12/2017 16:33:25
 *  Last Modified: 05/16/2017 18:44:19
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
#include <string>
#include <cstdint>

struct ServerEnv
{
    bool MIR2X_PRINT_ACTORPOD_FORWARD;
    bool MIR2X_PRINT_ACTOR_MESSAGE_COUNT;

    ServerEnv()
    {
        MIR2X_PRINT_ACTORPOD_FORWARD    = false;
        MIR2X_PRINT_ACTOR_MESSAGE_COUNT = false;
    }
};

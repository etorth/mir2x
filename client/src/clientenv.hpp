/*
 * =====================================================================================
 *
 *       Filename: clientenv.hpp
 *        Created: 05/12/2017 16:33:25
 *  Last Modified: 12/10/2017 01:31:53
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

struct ClientEnv
{
    std::string DebugArgs;

    bool TraceMove;


    int  MIR2X_DEBUG;
    bool MIR2X_DEBUG_SHOW_MAP_GRID;
    bool MIR2X_DEBUG_SHOW_LOCATION;
    bool MIR2X_DEBUG_SHOW_CREATURE_COVER;

    ClientEnv()
        : DebugArgs([]() -> std::string
          {
              if(auto pDebugArgs = std::getenv("MIR2X_DEBUG_ARGS")){
                  return std::string(pDebugArgs);
              }else{
                  return std::string("");
              }
          }())
        , TraceMove(CheckBoolArg("--trace-move"))
    {
        MIR2X_DEBUG = std::getenv("MIR2X_DEBUG") ? std::atoi(std::getenv("MIR2X_DEBUG")) : 0;

        MIR2X_DEBUG_SHOW_MAP_GRID       = (MIR2X_DEBUG >= 5) ? true : (std::getenv("MIR2X_DEBUG_SHOW_MAP_GRID"       ) ? true : false);
        MIR2X_DEBUG_SHOW_LOCATION       = (MIR2X_DEBUG >= 5) ? true : (std::getenv("MIR2X_DEBUG_SHOW_LOCATION"       ) ? true : false);
        MIR2X_DEBUG_SHOW_CREATURE_COVER = (MIR2X_DEBUG >= 5) ? true : (std::getenv("MIR2X_DEBUG_SHOW_CREATURE_COVER" ) ? true : false);
    }

    bool CheckBoolArg(const std::string &szArgName)
    {
        return DebugArgs.find(szArgName) != std::string::npos;
    }
};

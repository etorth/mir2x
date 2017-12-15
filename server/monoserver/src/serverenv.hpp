/*
 * =====================================================================================
 *
 *       Filename: serverenv.hpp
 *        Created: 05/12/2017 16:33:25
 *  Last Modified: 12/14/2017 18:59:08
 *
 *    Description:
 *
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
#include <string>
#include <cstdint>

struct ServerEnv
{
    const std::string DebugArgs;
    const bool DisableMapScript;        // "--disable-map-script"
    const bool TraceActorMessage;       // "--trace-actor-message"
    const bool TraceActorMessageCount;  // "--trace-actor-message-count"

    ServerEnv()
        : DebugArgs([]() -> std::string
          {
              if(auto pDebugArgs = std::getenv("MIR2X_DEBUG_ARGS")){
                  return std::string(pDebugArgs);
              }else{
                  return std::string("");
              }
          }())
        , DisableMapScript(CheckBoolArg("--disable-map-script"))
        , TraceActorMessage(CheckBoolArg("--trace-actor-message"))
        , TraceActorMessageCount(CheckBoolArg("--trace-actor-message-count"))
    {}

    bool CheckBoolArg(const std::string &szArgName)
    {
        return DebugArgs.find(szArgName) != std::string::npos;
    }
};

/*
 * =====================================================================================
 *
 *       Filename: serverenv.hpp
 *        Created: 05/12/2017 16:33:25
 *  Last Modified: 12/25/2017 19:16:44
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
        if(szArgName.empty()){
            return false;
        }

        size_t nPos = 0;
        while(true){
            nPos = DebugArgs.find(szArgName, nPos);
            if(nPos == std::string::npos){
                return false;
            }

            // if found, we need to make sure
            auto pEnd = DebugArgs.begin() + nPos + szArgName.size();
            if(pEnd == DebugArgs.end()){
                return true;
            }

            switch(*pEnd){
                case ' ' :
                case '\0':
                case '\t':
                case '\n':
                    {
                        return true;
                    }
                default:
                    {
                        break;
                    }
            }

            // continue to check
            nPos += szArgName.size();
        }

        return false;
    }
};

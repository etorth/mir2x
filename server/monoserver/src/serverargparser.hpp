/*
 * =====================================================================================
 *
 *       Filename: serverargparser.hpp
 *        Created: 11/26/2018 07:34:22
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
#include "typecast.hpp"
#include "fflerror.hpp"
#include "argparser.hpp"

struct ServerArgParser
{
    const bool DisableMapScript;        // "--disable-map-script"
    const bool TraceActorMessage;       // "--trace-actor-message"
    const bool TraceActorMessageCount;  // "--trace-actor-message-count"
    const int  actorPoolThread;         // "--actor-pool-thread"
    const int  uidGroup;                // "--uid-group"

    ServerArgParser(const argh::parser &cmdParser)
        : DisableMapScript(cmdParser["disable-map-script"])
        , TraceActorMessage(cmdParser["trace-actor-message"])
        , TraceActorMessageCount(cmdParser["trace-actor-message-count"])
        , actorPoolThread([&cmdParser]()
          {
              if(auto szThreadNum = cmdParser("actor-pool-thread").str(); !szThreadNum.empty()){
                  try{
                      return std::stoi(szThreadNum);
                  }catch(...){
                      return 1;
                  }
              }
              return 1;
          }())
        , uidGroup([&cmdParser]() -> int
          {
              if(const auto uidGroupStr = cmdParser("uid-group").str(); !uidGroupStr.empty()){
                  try{
                      return std::stoi(uidGroupStr);
                  }
                  catch(...){
                      throw fflerror("invalid option value for uid-group: %s", to_cstr(uidGroupStr));
                  }
              }
              return 17;
          }())
    {}
};

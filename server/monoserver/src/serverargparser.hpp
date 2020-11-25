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
#include "totype.hpp"
#include "fflerror.hpp"
#include "argparser.hpp"

struct ServerArgParser
{
    const bool disableProfiler;         // "--disable-profiler"
    const bool DisableMapScript;        // "--disable-map-script"
    const bool TraceActorMessage;       // "--trace-actor-message"
    const bool TraceActorMessageCount;  // "--trace-actor-message-count"
    const bool preloadMap;              // "--preload-map"
    const int  actorPoolThread;         // "--actor-pool-thread"

    ServerArgParser(const argh::parser &cmdParser)
        : disableProfiler(cmdParser["disable-profiler"])
        , DisableMapScript(cmdParser["disable-map-script"])
        , TraceActorMessage(cmdParser["trace-actor-message"])
        , TraceActorMessageCount(cmdParser["trace-actor-message-count"])
        , preloadMap(cmdParser["preload-map"])
        , actorPoolThread([&cmdParser]()
          {
              if(const auto numStr = cmdParser("actor-pool-thread").str(); !numStr.empty()){
                  try{
                      return std::stoi(numStr);
                  }
                  catch(...){
                      return 1;
                  }
              }
              return 1;
          }())
    {}
};

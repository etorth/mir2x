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
#include <thread>
#include <cstdint>
#include "totype.hpp"
#include "fflerror.hpp"
#include "argparser.hpp"

struct ServerArgParser
{
    const bool disableProfiler;         // "--disable-profiler"
    const bool DisableMapScript;        // "--disable-map-script"
    const bool traceActorMessage;       // "--trace-actor-message"
    const bool traceActorMessageCount;  // "--trace-actor-message-count"
    const bool disablePetSpawn;         // "--disable-pet-spawn"
    const bool disableMonsterSpawn;     // "--disable-monster-spawn"
    const bool preloadMap;              // "--preload-map"
    const int  actorPoolThread;         // "--actor-pool-thread"

    ServerArgParser(const argh::parser &cmdParser)
        : disableProfiler(cmdParser["disable-profiler"])
        , DisableMapScript(cmdParser["disable-map-script"])
        , traceActorMessage(cmdParser["trace-actor-message"])
        , traceActorMessageCount(cmdParser["trace-actor-message-count"])
        , disablePetSpawn(cmdParser["disable-pet-spawn"])
        , disableMonsterSpawn(cmdParser["disable-monster-spawn"])
        , preloadMap(cmdParser["preload-map"])
        , actorPoolThread([&cmdParser]() -> int
          {
              if(const auto numStr = cmdParser("actor-pool-thread").str(); !numStr.empty()){
                  try{
                      return std::stoi(numStr);
                  }
                  catch(...){
                      return 1;
                  }
              }
              else if(const auto hw = std::thread::hardware_concurrency(); hw > 0){
                  return hw;
              }
              else{
                  return 4;
              }
          }())
    {}
};

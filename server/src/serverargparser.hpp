#pragma once
#include <thread>
#include <cstdint>
#include "totype.hpp"
#include "fflerror.hpp"
#include "argparser.hpp"
#include "dbcomid.hpp"

struct ServerArgParser
{
    const bool disableProfiler;             // "--disable-profiler"
    const bool disableMapScript;            // "--disable-map-script"
    const bool disableQuestScript;          // "--disable-quest-script"

    const bool disableLearnMagicCheckJob;   // "--disable-learn-magic-check-job"
    const bool disableLearnMagicCheckLevel; // "--disable-learn-magic-check-level"
    const bool disableLearnMagicCheckPrior; // "--disable-learn-magic-check-prior"

    const bool traceActorMessage;           // "--trace-actor-message"
    const bool traceActorMessageCount;      // "--trace-actor-message-count"
    const bool enableUniqueActorMessageID;  // "--enable-unique-actor-message-id"

    const bool disablePetSpawn;             // "--disable-pet-spawn"
    const bool disableGuardSpawn;           // "--disable-guard-spawn"
    const bool disableMonsterSpawn;         // "--disable-monster-spawn"
    const bool disableNPCSpawn;             // "--disable-npc-spawn"

    const bool forceMonsterRandomMove;      // "--force-monster-random-move"
    const bool showStrikeGrid;              // "--show-strike-grid"
    const bool preloadMap;                  // "--preload-map"
    const bool autoLaunch;                  // "--auto-launch"
    const int  preloadMapID;                // "--preload-map-id"
    const int  actorPoolThread;             // "--actor-pool-thread"
    const int  logicalFPS;                  // "--logical-fps"
    const int  summonCount;                 // "--summon-count"
    const int  textFont;                    // "--text-font"

    const std::string loadSingleQuest;      // "--load-single-quest"

    ServerArgParser(const argh::parser &cmdParser)
        : disableProfiler(cmdParser["disable-profiler"])
        , disableMapScript(cmdParser["disable-map-script"])
        , disableQuestScript(cmdParser["disable-quest-script"])
        , disableLearnMagicCheckJob(cmdParser["disable-learn-magic-check-job"])
        , disableLearnMagicCheckLevel(cmdParser["disable-learn-magic-check-level"])
        , disableLearnMagicCheckPrior(cmdParser["disable-learn-magic-check-prior"])
        , traceActorMessage(cmdParser["trace-actor-message"])
        , traceActorMessageCount(cmdParser["trace-actor-message-count"])
        , enableUniqueActorMessageID(cmdParser["enable-unique-actor-message-id"])
        , disablePetSpawn(cmdParser["disable-pet-spawn"])
        , disableGuardSpawn(cmdParser["disable-guard-spawn"])
        , disableMonsterSpawn(cmdParser["disable-monster-spawn"])
        , disableNPCSpawn(cmdParser["disable-npc-spawn"])
        , forceMonsterRandomMove(cmdParser["force-monster-random-move"])
        , showStrikeGrid(cmdParser["show-strike-grid"])
        , preloadMap(cmdParser["preload-map"])
        , autoLaunch(cmdParser["auto-launch"])
        , preloadMapID([&cmdParser]() -> int
          {
              if(const auto s = cmdParser("preload-map-id").str(); !s.empty()){
                  const auto mapID = [&s]() -> uint32_t
                  {
                      try{
                          return to_u32(std::stoi(s));
                      }
                      catch(...){
                          return 0;
                      }
                  }();

                  if(mapID > 0){
                      if(DBCOM_MAPRECORD(mapID)){
                          return mapID;
                      }
                      else{
                          throw fflerror("no valid map for mapID: %llu", to_llu(mapID));
                      }
                  }

                  if(const auto mapIDFromName = DBCOM_MAPID(to_u8cstr(s)); mapIDFromName > 0){
                      return mapIDFromName;
                  }
                  throw fflerror("failed to parse mapID: %s", to_cstr(s));
              }
              return 0;
          }())
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
        , logicalFPS([&cmdParser]() -> int
          {
              if(const auto numStr = cmdParser("logical-fps").str(); !numStr.empty()){
                  try{
                      if(const auto fps = std::stoi(numStr); fps > 0){
                          return fps;
                      }
                  }
                  catch(...){
                      // ...
                  }
                  throw fflerror("invalid fps: %s", numStr.c_str());
              }
              else{
                  return 10;
              }
          }())
        , summonCount([&cmdParser]() -> int
          {
              if(const auto numStr = cmdParser("summon-count").str(); !numStr.empty()){
                  try{
                      if(const auto count = std::stoi(numStr); count > 0){
                          return count;
                      }
                  }
                  catch(...){
                      // ...
                  }
                  throw fflerror("invalid count: %s", numStr.c_str());
              }
              else{
                  return 1;
              }
          }())
        , textFont([&cmdParser]() -> int
          {
              if(const auto numStr = cmdParser("text-font").str(); !numStr.empty()){
                  try{
                      if(const auto font = std::stoi(numStr); font >= 0){
                          return font;
                      }
                  }
                  catch(...){
                      // ...
                  }
                  throw fflerror("invalid font: %s", numStr.c_str());
              }
              else{
                  return 0;
              }
          }())
        , loadSingleQuest(cmdParser("load-single-quest").str())
    {}
};

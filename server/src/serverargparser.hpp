#pragma once
#include <thread>
#include <cstdint>
#include "totype.hpp"
#include "fflerror.hpp"
#include "argf.hpp"
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
    const bool slave;                       // "--slave"
    const bool lightMasterServer;           // "--light-master-server"
    const int  preloadMapID;                // "--preload-map-id"
    const int  actorPoolThread;             // "--actor-pool-thread"
    const int  logicalFPS;                  // "--logical-fps"
    const int  summonCount;                 // "--summon-count"
    const int  textFont;                    // "--text-font"

    const std::string loadSingleQuest;      // "--load-single-quest"

    const std::string masterIP;             // "--master-ip"
    const std::pair<int, bool> masterPort;  // "--master-port"

    const std::pair<int, bool> clientPort;  // "--master-port"
    const std::pair<int, bool> peerPort;    // "--peer-port"

    struct defVals
    {
        constexpr static uint32_t masterPort = 6000;
        constexpr static uint32_t clientPort = 7000;
        constexpr static uint32_t   peerPort = 8000;
    };

    ServerArgParser(const argf::parser &cmdParser)
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
        , slave(cmdParser["slave"])
        , lightMasterServer(cmdParser["light-master-server"])
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

        , actorPoolThread(argf::parseInteger<int>(parseString(cmdParser, "actor-pool-thread", this->slave, argf::OPT, argf::OPT, false, false), "actor pool thread", argf::checkPositive,   std::max<int>(16, std::thread::hardware_concurrency())).first)
        , logicalFPS     (argf::parseInteger<int>(parseString(cmdParser, "logical-fps",       this->slave, argf::OPT, argf::BAN, false, false), "logical fps",       argf::checkPositive,   10).first)
        , summonCount    (argf::parseInteger<int>(parseString(cmdParser, "summon-count",      this->slave, argf::OPT, argf::BAN, false, false), "summon count",      argf::checkPositive,    1).first)
        , textFont       (argf::parseInteger<int>(parseString(cmdParser, "text-font",         this->slave, argf::OPT, argf::BAN, false, false), "text-font",         argf::checkNonNegative, 0).first)

        , loadSingleQuest(parseString(cmdParser, "load-single-quest", this->slave, argf::OPT, argf::BAN, false, false).value_or(std::string{}))
        , masterIP       (parseString(cmdParser, "master-ip",         this->slave, argf::BAN, argf::REQ, false, false).value_or(std::string{}))

        , masterPort(argf::parseInteger<int>(parseString(cmdParser, "master-port", this->slave, argf::BAN, argf::OPT, false, false), "master port", argf::checkUserPort, defVals::masterPort))
        , clientPort(argf::parseInteger<int>(parseString(cmdParser, "client-port", this->slave, argf::OPT, argf::BAN, false, false), "client port", argf::checkUserPort, defVals::clientPort))
        ,   peerPort(argf::parseInteger<int>(parseString(cmdParser,   "peer-port", this->slave, argf::OPT, argf::OPT, false, false),   "peer port", argf::checkUserPort, defVals::  peerPort))
    {}

    static std::optional<std::string> parseString(const argf::parser &parser, const std::string &opt,
            bool slave,                 // current mode

            int optInMaster,            // req/opt/ban in master
            int optInSlave,             // req/opt/ban in slave

            bool allowEmptyInMaster,    // option value can be empty in master
            bool allowEmptyInSlave)     // option value can be empty in slave
    {
        fflassert(str_haschar(opt));
        const auto optVal = parser.get_option(opt);

        switch(slave ? optInSlave : optInMaster){
            case argf::REQ:
                {
                    if(!optVal.has_value()){
                        throw fflerror("missing required option in %s mode: --%s", slave ? "slave" : "master", opt.c_str());
                    }
                    break;
                }
            case argf::BAN:
                {
                    if(optVal.has_value()){
                        throw fflerror("invalid option in %s mode: --%s", slave ? "slave" : "master", opt.c_str());
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }

        if(optVal.has_value() && optVal.value().empty() && !(slave ? allowEmptyInSlave : allowEmptyInMaster)){
            throw fflerror("invalid empty option value in %s mode: --%s", slave ? "slave" : "master", opt.c_str());
        }
        return optVal;
    }
};

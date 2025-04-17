#pragma once
#include <thread>
#include <cstdint>
#include "totype.hpp"
#include "fflerror.hpp"
#include "argf.hpp"
#include "dbcomid.hpp"
#include "serdesmsg.hpp"

struct ServerArgParser
{
    // ** specified by master and slave
    // ** both sides need

    const bool slave;                           // "--slave"
    const bool disableProfiler;                 // "--disable-profiler"

    const int actorPoolThread;                  // "--actor-pool-thread"
    const std::pair<int, bool> peerPort;        // "--peer-port"

    // ** specified by slave only
    // ** master doesn't need

    const std::string masterIP;                 // "--master-ip"
    const std::pair<int, bool> masterPort;      // "--master-port"

    // ** specified by master only
    // ** slave doesn't need

    const int textFont;                         // "--text-font"
    const bool autoLaunch;                      // "--auto-launch"

    const std::pair<int, bool> clientPort;      // "--client-port"

    const std::pair<int, bool> preloadMap;      // "--preload-map"
    const std::string          loadSingleQuest; // "--load-single-quest"

    // ** specified by master only
    // ** forward to slave side, slave needs

    const int logicalFPS;                       // "--logical-fps"
    const int summonCount;                      // "--summon-count"

    const bool lightMasterServer;               // "--light-master-server"

    const bool disableMapScript;                // "--disable-map-script"
    const bool disableQuestScript;              // "--disable-quest-script"

    const bool disableLearnMagicCheckJob;       // "--disable-learn-magic-check-job"
    const bool disableLearnMagicCheckLevel;     // "--disable-learn-magic-check-level"
    const bool disableLearnMagicCheckPrior;     // "--disable-learn-magic-check-prior"

    const bool traceActorMessage;               // "--trace-actor-message"
    const bool traceActorMessageCount;          // "--trace-actor-message-count"
    const bool enableUniqueActorMessageID;      // "--enable-unique-actor-message-id"

    const bool disablePetSpawn;                 // "--disable-pet-spawn"
    const bool disableGuardSpawn;               // "--disable-guard-spawn"
    const bool disableMonsterSpawn;             // "--disable-monster-spawn"
    const bool disableNPCSpawn;                 // "--disable-npc-spawn"

    const bool showStrikeGrid;                  // "--show-strike-grid"
    const bool forceMonsterRandomMove;          // "--force-monster-random-move"

    ServerArgParser(const argf::parser &parser)
        // master & slave
        : slave          (argf::parseInteger<bool>(parseString(parser, "--slave",             true,        argf::BAN, argf::OPT, true, true), "slave",             argf::checkPass<bool>,           false, true).first)
        , disableProfiler(argf::parseInteger<bool>(parseString(parser, "--disable-profiler",  this->slave, argf::OPT, argf::OPT, true, true), "disable-profiler",  argf::checkPass<bool>,           false, true).first)
        , actorPoolThread(argf::parseInteger<int> (parseString(parser, "--actor-pool-thread", this->slave, argf::OPT, argf::OPT            ), "actor-pool-thread", argf::checkPositive,             cpuCount() ).first)
        ,        peerPort(argf::parseInteger<int> (parseString(parser, "--peer-port",         this->slave, argf::OPT, argf::OPT            ), "peer-port",         argf::checkUserListenPort(true), this->slave ? 0 : argf::defVal::masterPeerPort))

        // slave only
        , masterIP  (                        parseString(parser, "--master-ip",   this->slave, argf::BAN, argf::REQ).value_or(std::string{}))
        , masterPort(argf::parseInteger<int>(parseString(parser, "--master-port", this->slave, argf::BAN, argf::OPT), "master-port", argf::checkUserListenPort(false), argf::defVal::masterPeerPort))

        // master only
        , textFont       (argf::parseInteger<int> (parseString(parser, "--text-font",         this->slave, argf::OPT, argf::BAN            ), "text-font",   argf::checkNonNegative,           0          ).first)
        , autoLaunch     (argf::parseInteger<bool>(parseString(parser, "--auto-launch",       this->slave, argf::OPT, argf::BAN, true, true), "auto-launch", argf::checkPass<bool>,            false, true).first)
        , clientPort     (argf::parseInteger<int> (parseString(parser, "--client-port",       this->slave, argf::OPT, argf::BAN            ), "client-port", argf::checkUserListenPort(false), argf::defVal::clientPort))
        , preloadMap     (argf::parseMapIDString  (parseString(parser, "--preload-map",       this->slave, argf::OPT, argf::BAN, true      ), "preload-map", true, 0, 0))
        , loadSingleQuest(                         parseString(parser, "--load-single-quest", this->slave, argf::OPT, argf::BAN            ).value_or(std::string{}))

        // master only, forward to slave
        , logicalFPS                 (argf::parseInteger<int> (parseString(parser, "--logical-fps",                     this->slave, argf::OPT, argf::BAN            ), "logical-fps",                     argf::checkPositive,            10).first)
        , summonCount                (argf::parseInteger<int> (parseString(parser, "--summon-count",                    this->slave, argf::OPT, argf::BAN            ), "summon-count",                    argf::checkPositive,             1).first)
        , lightMasterServer          (argf::parseInteger<bool>(parseString(parser, "--light-master-server",             this->slave, argf::OPT, argf::BAN, true, true), "light-master-server",             argf::checkPass<bool>, false, true).first)
        , disableMapScript           (argf::parseInteger<bool>(parseString(parser, "--disable-map-script",              this->slave, argf::OPT, argf::BAN, true, true), "disable-map-script",              argf::checkPass<bool>, false, true).first)
        , disableQuestScript         (argf::parseInteger<bool>(parseString(parser, "--disable-quest-script",            this->slave, argf::OPT, argf::BAN, true, true), "disable-quest-script",            argf::checkPass<bool>, false, true).first)
        , disableLearnMagicCheckJob  (argf::parseInteger<bool>(parseString(parser, "--disable-learn-magic-check-job",   this->slave, argf::OPT, argf::BAN, true, true), "disable-learn-magic-check-job",   argf::checkPass<bool>, false, true).first)
        , disableLearnMagicCheckLevel(argf::parseInteger<bool>(parseString(parser, "--disable-learn-magic-check-level", this->slave, argf::OPT, argf::BAN, true, true), "disable-learn-magic-check-level", argf::checkPass<bool>, false, true).first)
        , disableLearnMagicCheckPrior(argf::parseInteger<bool>(parseString(parser, "--disable-learn-magic-check-prior", this->slave, argf::OPT, argf::BAN, true, true), "disable-learn-magic-check-prior", argf::checkPass<bool>, false, true).first)
        , traceActorMessage          (argf::parseInteger<bool>(parseString(parser, "--trace-actor-message",             this->slave, argf::OPT, argf::BAN, true, true), "trace-actor-message",             argf::checkPass<bool>, false, true).first)
        , traceActorMessageCount     (argf::parseInteger<bool>(parseString(parser, "--trace-actor-message-count",       this->slave, argf::OPT, argf::BAN, true, true), "trace-actor-message-count",       argf::checkPass<bool>, false, true).first)
        , enableUniqueActorMessageID (argf::parseInteger<bool>(parseString(parser, "--enable-unique-actor-message-id",  this->slave, argf::OPT, argf::BAN, true, true), "enable-unique-actor-message-id",  argf::checkPass<bool>, false, true).first)
        , disablePetSpawn            (argf::parseInteger<bool>(parseString(parser, "--disable-pet-spawn",               this->slave, argf::OPT, argf::BAN, true, true), "disable-pet-spawn",               argf::checkPass<bool>, false, true).first)
        , disableGuardSpawn          (argf::parseInteger<bool>(parseString(parser, "--disable-guard-spawn",             this->slave, argf::OPT, argf::BAN, true, true), "disable-guard-spawn",             argf::checkPass<bool>, false, true).first)
        , disableMonsterSpawn        (argf::parseInteger<bool>(parseString(parser, "--disable-monster-spawn",           this->slave, argf::OPT, argf::BAN, true, true), "disable-monster-spawn",           argf::checkPass<bool>, false, true).first)
        , disableNPCSpawn            (argf::parseInteger<bool>(parseString(parser, "--disable-npc-spawn",               this->slave, argf::OPT, argf::BAN, true, true), "disable-npc-spawn",               argf::checkPass<bool>, false, true).first)
        , showStrikeGrid             (argf::parseInteger<bool>(parseString(parser, "--show-strike-grid",                this->slave, argf::OPT, argf::BAN, true, true), "show-strike-grid",                argf::checkPass<bool>, false, true).first)
        , forceMonsterRandomMove     (argf::parseInteger<bool>(parseString(parser, "--force-monster-random-move",       this->slave, argf::OPT, argf::BAN, true, true), "force-monster-random-move",       argf::checkPass<bool>, false, true).first)
    {}

    const char *runMode(bool cap = false) const
    {
        return slave ? (cap ? "Slave" : "slave") : (cap ? "Master" : "master");
    }

    bool preloadMapCheck(uint32_t mapID) const
    {
        if(preloadMap.second){
            return false;
        }

        if(preloadMap.first){
            return preloadMap.first == to_d(mapID);
        }

        return true;
    }

    static int cpuCount()
    {
        if(const auto c = std::thread::hardware_concurrency(); c > 0){
            return static_cast<int>(c);
        }
        else{
            return 16;
        }
    }

    static std::optional<std::string> parseString(const argf::parser &parser, const std::string &opt,
            bool slave,         // current mode

            int optInMaster,    // req/opt/ban in master
            int optInSlave,     // req/opt/ban in slave

            bool allowEmptyInMaster = false, // option value can be empty in master
            bool allowEmptyInSlave  = false) // option value can be empty in slave
    {
        fflassert(str_haschar(opt));
        const auto optVal = parser.get_option(opt);

        switch(slave ? optInSlave : optInMaster){
            case argf::REQ:
                {
                    if(!optVal.has_value()){
                        throw fflerror("missing required option in %s mode: %s", slave ? "slave" : "master", opt.c_str());
                    }
                    break;
                }
            case argf::BAN:
                {
                    if(optVal.has_value()){
                        throw fflerror("invalid option in %s mode: %s", slave ? "slave" : "master", opt.c_str());
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }

        if(optVal.has_value() && optVal.value().empty() && !(slave ? allowEmptyInSlave : allowEmptyInMaster)){
            throw fflerror("invalid empty option value in %s mode: %s", slave ? "slave" : "master", opt.c_str());
        }
        return optVal;
    }

    void getPeerConfig(SDPeerConfig *confg) const
    {
        if(confg){
            confg->logicalFPS                  = this->logicalFPS;
            confg->summonCount                 = this->summonCount;
            confg->lightMasterServer           = this->lightMasterServer;
            confg->disableMapScript            = this->disableMapScript;
            confg->disableQuestScript          = this->disableQuestScript;
            confg->disableLearnMagicCheckJob   = this->disableLearnMagicCheckJob;
            confg->disableLearnMagicCheckLevel = this->disableLearnMagicCheckLevel;
            confg->disableLearnMagicCheckPrior = this->disableLearnMagicCheckPrior;
            confg->traceActorMessage           = this->traceActorMessage;
            confg->traceActorMessageCount      = this->traceActorMessageCount;
            confg->enableUniqueActorMessageID  = this->enableUniqueActorMessageID;
            confg->disablePetSpawn             = this->disablePetSpawn;
            confg->disableGuardSpawn           = this->disableGuardSpawn;
            confg->disableMonsterSpawn         = this->disableMonsterSpawn;
            confg->disableNPCSpawn             = this->disableNPCSpawn;
            confg->showStrikeGrid              = this->showStrikeGrid;
            confg->forceMonsterRandomMove      = this->forceMonsterRandomMove;
        }
    }
};

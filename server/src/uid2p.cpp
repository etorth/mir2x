#include "uidf.hpp"
#include "uid2p.hpp"
#include "actorpool.hpp"
#include "serverargparser.hpp"

extern ActorPool *g_actorPool;
extern ServerArgParser *g_serverArgParser;

size_t uid2p::peerIndex(uint64_t uid)
{
    if(const auto peerCount = g_actorPool->hasPeer(); peerCount > 0){
        switch(uidf::getUIDType(uid)){
            case UID_MAP:
            case UID_MON:
                {
                    if(g_serverArgParser->lightMasterServer){
                        return (uid % peerCount) + 1;
                    }
                    else{
                        return uid % (peerCount + 1);
                    }
                }
            default:
                {
                    break;
                }
        }
    }
    return 0;
}

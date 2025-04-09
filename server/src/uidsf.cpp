#include "uidf.hpp"
#include "mathf.hpp"
#include "uidsf.hpp"
#include "fflerror.hpp"
#include "actorpool.hpp"
#include "serverargparser.hpp"

extern ActorPool *g_actorPool;
extern ServerArgParser *g_serverArgParser;

size_t uidsf::peerCount()
{
    return g_actorPool->peerCount();
}

size_t uidsf::peerIndex()
{
    return g_actorPool->peerIndex();
}

size_t uidsf::pickPeerIndex(int uidType, std::optional<size_t> seedOpt)
{
    switch(uidType){
        case UID_MAP:
            {
                const size_t peerCount = uidsf::peerCount();
                const size_t seed = seedOpt.value_or(mathf::rand());

                if(peerCount > 0){
                    if(g_serverArgParser->lightMasterServer){
                        return (seed % peerCount) + 1;
                    }
                    else{
                        return seed % (peerCount + 1);
                    }
                }
                else{
                    return 0;
                }
            }
        default:
            {
                throw fflerror("invalid uid type %d", uidType);
            }
    }
}

bool uidsf::isLocalUID(uint64_t uid)
{
    return uidf::peerIndex(uid) == uidsf::peerIndex();
}

uint64_t uidsf::getMapBaseUID(uint32_t mapID)
{
    return uidf::getMapBaseUID(mapID, uidsf::pickPeerIndex(UID_MAP, mapID));
}

uint64_t uidsf::getPeerCoreUID()
{
    return uidf::getPeerCoreUID(uidsf::peerIndex());
}

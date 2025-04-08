#include "uidf.hpp"
#include "uidsf.hpp"
#include "fflerror.hpp"
#include "actorpool.hpp"
#include "serverargparser.hpp"

extern ActorPool *g_actorPool;
extern ServerArgParser *g_serverArgParser;

bool uidsf::isLocalUID(uint64_t uid)
{
    return uidsf::peerIndex(uid) == g_actorPool->peerIndex();
}

size_t uidsf::peerIndex(uint64_t uid)
{
    if(const auto peerCount = g_actorPool->hasPeer(); peerCount > 0){
        switch(uidf::getUIDType(uid)){
            case UID_COR:
                {
                    if(const auto index = uidf::getServiceCoreSeq(uid); index <= peerCount){
                        return index;
                    }
                    else{
                        throw fflerror("invalid service core seq: %zu", to_uz(index));
                    }
                }
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

uint64_t uidsf::getServiceCoreUID()
{
    return uidf::getServiceCoreUID(g_actorPool->peerIndex());
}

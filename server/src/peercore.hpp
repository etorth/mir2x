#pragma once
#include <string>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include "serdesmsg.hpp"
#include "enableaddco.hpp"
#include "serverobject.hpp"

class PeerCore: public ServerObject
{
    protected:
        std::unique_ptr<EnableAddCO> m_addCO;

    protected:
        std::unordered_set<uint64_t> m_mapList;

    public:
        PeerCore();
        ~PeerCore() = default;

    protected:
        corof::entrance onActivate() override
        {
            ServerObject::onActivate();
            m_addCO = std::make_unique<EnableAddCO>(m_actorPod);
            return {};
        }

    public:
        std::pair<bool, bool> loadMap(uint64_t); // {loadOK, newLoad}

    protected:
        corof::entrance onActorMsg(const ActorMsgPack &) override;

    protected:
        corof::entrance on_AM_PEERCONFIG (const ActorMsgPack &);
        corof::entrance on_AM_PEERLOADMAP(const ActorMsgPack &);
};

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
        void onActivate() override
        {
            ServerObject::onActivate();
            m_addCO = std::make_unique<EnableAddCO>(m_actorPod);
        }

    public:
        std::pair<bool, bool> loadMap(uint64_t); // {loadOK, newLoad}

    protected:
        void operateAM(const ActorMsgPack &) override;

    protected:
        void on_AM_PEERCONFIG (const ActorMsgPack &);
        void on_AM_PEERLOADMAP(const ActorMsgPack &);
};

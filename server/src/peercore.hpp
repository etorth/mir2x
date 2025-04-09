#pragma once
#include <string>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include "serdesmsg.hpp"
#include "serverobject.hpp"

class CharObject;
class ServerGuard;
class Player;
class NPChar;
class Monster;
class PeerCore: public ServerObject
{
    protected:
        std::unordered_set<uint64_t> m_mapList;

    public:
        PeerCore();
        ~PeerCore() = default;

    public:
        std::pair<bool, bool> loadMap(uint64_t); // {loadOK, newLoad}

    protected:
        void operateAM(const ActorMsgPack &) override;

    protected:
        bool adjustMapGLoc(uint32_t, int &, int &, bool);
        std::optional<std::pair<int, int>> getMapGLoc(uint32_t, int, int, std::optional<int> = std::nullopt) const;

    protected:
        CharObject *addCO(SDInitCharObject);

    private:
        ServerGuard *addGuard  (SDInitGuard  );
        Player      *addPlayer (SDInitPlayer );
        NPChar      *addNPChar (SDInitNPChar );
        Monster     *addMonster(SDInitMonster);

    protected:
        void on_AM_ADDCO      (const ActorMsgPack &);
        void on_AM_PEERLOADMAP(const ActorMsgPack &);
};

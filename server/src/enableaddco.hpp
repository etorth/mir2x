#pragma once
#include <cstdint>
#include <utility>
#include <optional>
#include "serdesmsg.hpp"

class ActorPod;
class CharObject;
class ServerGuard;
class Player;
class NPChar;
class Monster;
class EnableAddCO
{
    private:
        ActorPod * m_actorPod;

    public:
        explicit EnableAddCO(ActorPod *);

    protected:
        bool adjustMapGLoc(uint32_t, int &, int &, bool);
        std::optional<std::pair<int, int>> getMapGLoc(uint32_t, int, int, std::optional<int> = std::nullopt) const;

    public:
        CharObject *addCO(SDInitCharObject);

    private:
        ServerGuard *addGuard  (SDInitGuard  );
        Player      *addPlayer (SDInitPlayer );
        NPChar      *addNPChar (SDInitNPChar );
        Monster     *addMonster(SDInitMonster);
};

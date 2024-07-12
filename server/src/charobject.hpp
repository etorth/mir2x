#pragma once
#include <memory>
#include <cstdint>
#include <unordered_map>
#include "servermap.hpp"
#include "actionnode.hpp"
#include "timedstate.hpp"
#include "protocoldef.hpp"
#include "serverobject.hpp"

struct COLocation
{
    uint64_t uid = 0;
    uint32_t mapID = 0;

    int x = -1;
    int y = -1;
    int direction = DIR_NONE;
};

class CharObject: public ServerObject
{
    protected:
        class LuaThreadRunner: public ServerObject::LuaThreadRunner
        {
            public:
                LuaThreadRunner(CharObject *);

            public:
                CharObject *getCO() const
                {
                    return static_cast<CharObject *>(getSO());
                }
        };

    protected:
        const ServerMap * m_map;

    protected:
        std::unordered_map<uint64_t, COLocation> m_inViewCOList;

    protected:
        TimedState<bool> m_dead;

    protected:
        int m_X;
        int m_Y;
        int m_direction;

    public:
        CharObject(
                const ServerMap *,  // server map
                uint64_t,           // uid
                int,                // map x
                int,                // map y
                int);               // direction

    public:
        ~CharObject() = default;

    public:
        int X() const
        {
            return m_X;
        }

        int Y() const
        {
            return m_Y;
        }

    public:
        int Direction() const
        {
            return m_direction;
        }

        uint32_t mapID() const
        {
            return m_map ? m_map->ID() : 0;
        }

        uint64_t mapUID() const
        {
            return m_map->UID();
        }

    public:
        virtual bool update() = 0;

    public:
        void onActivate() override
        {
            ServerObject::onActivate();
            dispatchAction(ActionSpawn
            {
                .direction = Direction(),
                .x = X(),
                .y = Y(),
            });
        }

    protected:
        virtual void reportCO(uint64_t) = 0;

    protected:
        virtual void dispatchAction(          const ActionNode &, bool = false);
        virtual void dispatchAction(uint64_t, const ActionNode &);

    protected:
        void getCOLocation(uint64_t, std::function<void(const COLocation &)>, std::function<void()> = []{});

    protected:
        void addMonster(uint32_t, int, int, bool);

    protected:
        virtual bool goDie()   = 0;
        virtual bool goGhost() = 0;

    protected:
        bool inView(uint32_t, int, int) const;

    protected:
        void trimInViewCO();
        int  updateInViewCO(const COLocation &, bool = false);
        void foreachInViewCO(std::function<void(const COLocation &)>);

    protected:
        COLocation *getInViewCOPtr(uint64_t uid)
        {
            if(auto p = m_inViewCOList.find(uid); p != m_inViewCOList.end()){
                return std::addressof(p->second);
            }
            return nullptr;
        }

        std::vector<uint64_t> getInViewUIDList() const
        {
            std::vector<uint64_t> uidList;
            uidList.reserve(m_inViewCOList.size());

            for(const auto &[uid, coLoc]: m_inViewCOList){
                uidList.push_back(uid);
            }
            return uidList;
        }

    protected:
        bool isNPChar() const
        {
            return uidf::isNPChar(UID());
        }

        bool isPlayer() const
        {
            return uidf::isPlayer(UID());
        }

        bool isMonster() const
        {
            return uidf::isMonster(UID());
        }

        bool isMonster(uint32_t monID) const
        {
            return uidf::isMonster(UID(), monID);
        }

        bool isMonster(const char8_t *monName) const
        {
            return uidf::isMonster(UID(), monName);
        }

    protected:
        void notifyDead(uint64_t);

    protected:
        virtual ActionNode makeActionStand() const;

    protected:
        template<typename... Args> void dispatchInViewCONetPackage(uint8_t type, Args && ... args)
        {
            for(const auto &[uid, coLoc]: m_inViewCOList){
                if(uidf::getUIDType(coLoc.uid) == UID_PLY){
                    forwardNetPackage(coLoc.uid, type, std::forward<Args>(args)...);
                }
            }
        }
};

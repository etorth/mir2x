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
    uint64_t mapUID = 0;

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
        uint64_t m_mapUID;
        std::shared_ptr<Mir2xMapData> m_mapBinPtr;

    protected:
        std::unordered_map<uint64_t, COLocation> m_inViewCOList;

    protected:
        int m_X;
        int m_Y;
        int m_direction;

    public:
        CharObject(
                uint64_t, // uid
                uint64_t, // server map uid
                int,      // map x
                int,      // map y
                int);     // direction

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

    public:
        uint32_t mapID() const
        {
            return uidf::getMapID(m_mapUID);
        }

        uint64_t mapUID() const
        {
            return m_mapUID;
        }

        const Mir2xMapData *mapBin() const
        {
            return m_mapBinPtr.get();
        }

    public:
        corof::awaitable<> onActivate() override
        {
            co_await ServerObject::onActivate();
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
        corof::awaitable<std::optional<COLocation>> getCOLocation(uint64_t);

    protected:
        bool inView(uint64_t, int, int) const;

    protected:
        void trimInViewCO();
        bool removeInViewCO(uint64_t);
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

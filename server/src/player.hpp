/*
 * =====================================================================================
 *
 *       Filename: player.hpp
 *        Created: 04/08/2016 22:37:01
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <set>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include "totype.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "combatnode.hpp"

class Player final: public CharObject
{
    private:
        friend class CharObject;

    protected:
        // empty: hasn't bind to a channel yet
        //  zero: has received AM_BADCHANNEL and need to go offine
        //  xxxx: channel id of associated working channel
        std::optional<uint32_t> m_channID;

    protected:
        uint32_t m_exp;

    protected:
        std::string m_name;
        uint32_t m_nameColor;

        uint32_t m_hair;
        uint32_t m_hairColor;

    protected:
        std::set<uint64_t> m_slaveList;

    private:
        bool m_pickUpLock = false;

    private:
        // flag to show next hit is 攻杀剑术
        // by implementation client reports action to server, at the same time client present action gfx without wait server's response
        // so for some random action like 攻杀技术, server has to send enable flag to client in advance to show that ``next hit you can use 攻杀技术"
        bool m_nextStrike = false;

    private:
        SDItemStorage      m_sdItemStorage;
        SDLearnedMagicList m_sdLearnedMagicList;
        SDRuntimeConfig    m_sdRuntimeConfig;

    public:
        Player(const SDInitPlayer &, const ServerMap *);

    public:
        ~Player() = default;

    protected:
        uint32_t exp() const
        {
            return m_exp;
        }

        uint32_t gold() const
        {
            return m_sdItemStorage.gold;
        }

        uint32_t level() const
        {
            return to_u32(SYS_LEVEL(exp()));
        }

    public:
        int Speed(int) const override
        {
            return 5;
        }

        bool update() override;

    protected:
        void operateNet(uint8_t, const uint8_t *, size_t);

    protected:
        void operateAM(const ActorMsgPack &);

    private:
        void on_AM_EXP(const ActorMsgPack &);
        void on_AM_MISS(const ActorMsgPack &);
        void on_AM_HEAL(const ActorMsgPack &);
        void on_AM_GIFT(const ActorMsgPack &);
        void on_AM_ACTION(const ActorMsgPack &);
        void on_AM_ATTACK(const ActorMsgPack &);
        void on_AM_OFFLINE(const ActorMsgPack &);
        void on_AM_CORECORD(const ActorMsgPack &);
        void on_AM_NPCQUERY(const ActorMsgPack &);
        void on_AM_METRONOME(const ActorMsgPack &);
        void on_AM_MAPSWITCHTRIGGER(const ActorMsgPack &);
        void on_AM_SENDPACKAGE(const ActorMsgPack &);
        void on_AM_RECVPACKAGE(const ActorMsgPack &);
        void on_AM_BADCHANNEL(const ActorMsgPack &);
        void on_AM_NOTIFYDEAD(const ActorMsgPack &);
        void on_AM_NOTIFYNEWCO(const ActorMsgPack &);
        void on_AM_QUERYHEALTH(const ActorMsgPack &);
        void on_AM_DEADFADEOUT(const ActorMsgPack &);
        void on_AM_BADACTORPOD(const ActorMsgPack &);
        void on_AM_BINDCHANNEL(const ActorMsgPack &);
        void on_AM_CHECKMASTER(const ActorMsgPack &);
        void on_AM_QUERYCORECORD(const ActorMsgPack &);
        void on_AM_QUERYLOCATION(const ActorMsgPack &);
        void on_AM_QUERYFRIENDTYPE(const ActorMsgPack &);
        void on_AM_REMOVEGROUNDITEM(const ActorMsgPack &);
        void on_AM_QUERYPLAYERWLDESP(const ActorMsgPack &);

    private:
        void net_CM_REQUESTADDEXP           (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTKILLPETS         (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTSPACEMOVE        (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTMAGICDAMAGE      (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTRETRIEVESECUREDITEM(uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYCORECORD           (uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYSELLITEMLIST       (uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYPLAYERWLDESP       (uint8_t, const uint8_t *, size_t);
        void net_CM_ACTION                  (uint8_t, const uint8_t *, size_t);
        void net_CM_PICKUP                  (uint8_t, const uint8_t *, size_t);
        void net_CM_PING                    (uint8_t, const uint8_t *, size_t);
        void net_CM_CONSUMEITEM             (uint8_t, const uint8_t *, size_t);
        void net_CM_BUY                     (uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYGOLD               (uint8_t, const uint8_t *, size_t);
        void net_CM_NPCEVENT                (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTEQUIPWEAR        (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTGRABWEAR         (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTEQUIPBELT        (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTGRABBELT         (uint8_t, const uint8_t *, size_t);
        void net_CM_DROPITEM                (uint8_t, const uint8_t *, size_t);
        void net_CM_SETMAGICKEY             (uint8_t, const uint8_t *, size_t);

    protected:
        void reportGold();
        void reportStand();
        void reportHealth();
        void reportNextStrike();
        void reportDeadUID(uint64_t);
        void reportCO(uint64_t);
        void reportOffline(uint64_t, uint32_t);
        void reportRemoveItem(uint32_t, uint32_t, size_t);
        void reportSecuredItemList();

    protected:
        virtual void reportAction(uint64_t, const ActionNode &);

    protected:
        void dispatchOffline();

    protected:
        bool struckDamage(const DamageNode &);

    protected:
        void RequestKillPets();

    protected:
        DamageNode getAttackDamage(int) const;

    protected:
        bool dcValid(int, bool);

    protected:
        bool ActionValid(const ActionNode &);
        bool MotionValid(const ActionNode &);

    protected:
        void onCMActionMove    (CMAction);
        void onCMActionStand   (CMAction);
        void onCMActionSpell   (CMAction);
        void onCMActionSpinKick(CMAction);
        void onCMActionAttack  (CMAction);

    private:
        void postNetMessage(uint8_t, const void *, size_t);

    private:
        void postNetMessage(uint8_t headCode)
        {
            postNetMessage(headCode, nullptr, 0);
        }

        void postNetMessage(uint8_t headCode, const std::string &buf)
        {
            postNetMessage(headCode, buf.data(), buf.length());
        }

        void postNetMessage(uint8_t headCode, const std::u8string &buf)
        {
            postNetMessage(headCode, buf.data(), buf.length());
        }

        template<typename T> void postNetMessage(uint8_t headCode, T& t)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            postNetMessage(headCode, &t, sizeof(t));
        }

    protected:
        virtual bool goDie();
        virtual bool goGhost();

    protected:
        bool goOffline();

    protected:
        virtual int MaxStep() const;

    protected:
        virtual void recoverHealth();

    protected:
        void gainExp(int);

    protected:
        bool CanPickUp(uint32_t, uint32_t);

    private:
        void dbUpdateExp();
        void dbUpdateMapGLoc();

    private:
        void dbLoadInventory();
        void dbUpdateInventoryItem(const SDItem &);
        void dbRemoveInventoryItem(const SDItem &);
        void dbRemoveInventoryItem(uint32_t, uint32_t);

    private:
        void dbSecureItem(uint32_t, uint32_t);
        SDItem dbRetrieveSecuredItem(uint32_t, uint32_t);
        std::vector<SDItem> dbLoadSecuredItemList() const;

    private:
        void dbLoadBelt();
        void dbUpdateBeltItem(size_t, const SDItem &);
        void dbRemoveBeltItem(size_t);

    private:
        void dbLoadWear();
        void dbUpdateWearItem(int, const SDItem &);
        void dbRemoveWearItem(int);

    private:
        void dbLoadRuntimeConfig();

    private:
        void dbLoadLearnedMagic();
        void dbUpdateMagicKey(uint32_t, char);

    private:
        void dbLearnMagic(uint32_t);
        void dbAddMagicExp(uint32_t, size_t);

    protected:
        void checkFriend(uint64_t, std::function<void(int)>) override;

    private:
        void postExp();

    private:
        void postOnlineOK();

    private:
        bool hasInventoryItem(uint32_t, uint32_t, size_t) const;
        const SDItem &addInventoryItem(SDItem, bool);

    private:
        size_t removeInventoryItem(const SDItem &);
        size_t removeInventoryItem(uint32_t, uint32_t);
        size_t removeInventoryItem(uint32_t, uint32_t, size_t);

    private:
        const SDItem &findInventoryItem(uint32_t, uint32_t) const;

    private:
        void addSecuredItem(uint32_t, uint32_t);
        void removeSecuredItem(uint32_t, uint32_t);

    private:
        size_t getGold() const
        {
            return m_sdItemStorage.gold;
        }

        void setGold(size_t);

    public:
        uint32_t dbid() const
        {
            return uidf::getPlayerDBID(UID());
        }

        bool gender() const
        {
            return uidf::getPlayerGender(UID());
        }

    private:
        void setWLItem(int, SDItem);

    private:
        SDItem createItem(uint32_t);

    private:
        bool canWear(uint32_t, int) const;

    private:
        static std::vector<std::string> parseNPCQuery(const char *);

    private:
        template<typename... Args> void dispatchNetPackage(bool sendSelf, uint8_t type, Args && ... args)
        {
            if(sendSelf){
                postNetMessage(type, std::forward<Args>(args)...);
            }
            dispatchInViewCONetPackage(type, std::forward<Args>(args)...);
        }

    public:
        void notifySlaveGLoc();
};

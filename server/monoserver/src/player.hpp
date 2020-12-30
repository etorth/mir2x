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
#include "monoserver.hpp"
#include "charobject.hpp"

class Player final: public CharObject
{
    private:
        friend class CharObject;

    protected:
        const uint32_t m_DBID;
        const uint32_t m_jobID;

    protected:
        uint32_t m_channID;

    protected:
        uint32_t m_exp;
        uint32_t m_level;

    protected:
        uint32_t m_gold;
        std::vector<CommonItem> m_inventory;

    protected:
        const std::string m_name;

    protected:
        std::set<uint64_t> m_slaveList;

    public:
        Player(uint32_t,                // DBID
                ServiceCore *,          //
                ServerMap *,            //
                int,                    // map x
                int,                    // map y
                int);                   // direction

    public:
        ~Player();

    protected:
        uint32_t Exp() const
        {
            return m_exp;
        }

        uint32_t Gold() const
        {
            return m_gold;
        }

        uint32_t Level() const
        {
            return m_level;
        }

        uint32_t DBID() const
        {
            return m_DBID;
        }

        uint32_t JobID() const
        {
            return m_jobID;
        }

        uint32_t ChannID()
        {
            return m_channID;
        }

    protected:
        bool sendNetBuf(uint8_t, const uint8_t *, size_t);

    protected:
        template<typename T> bool sendNet(uint8_t hc, const T &t)
        {
            return sendNetBuf(hc, (const uint8_t *)(&t), sizeof(t));
        }

    public:
        int Speed()
        {
            return 5;
        }

        bool update() override;

    protected:
        void operateNet(uint8_t, const uint8_t *, size_t);

    protected:
        void operateAM(const MessagePack &);

    private:
        void on_MPK_EXP(const MessagePack &);
        void on_MPK_MISS(const MessagePack &);
        void on_MPK_ACTION(const MessagePack &);
        void on_MPK_ATTACK(const MessagePack &);
        void on_MPK_OFFLINE(const MessagePack &);
        void on_MPK_CORECORD(const MessagePack &);
        void on_MPK_PICKUPOK(const MessagePack &);
        void on_MPK_UPDATEHP(const MessagePack &);
        void on_MPK_NPCQUERY(const MessagePack &);
        void on_MPK_METRONOME(const MessagePack &);
        void on_MPK_MAPSWITCH(const MessagePack &);
        void on_MPK_NETPACKAGE(const MessagePack &);
        void on_MPK_BADCHANNEL(const MessagePack &);
        void on_MPK_NOTIFYDEAD(const MessagePack &);
        void on_MPK_NOTIFYNEWCO(const MessagePack &);
        void on_MPK_DEADFADEOUT(const MessagePack &);
        void on_MPK_BADACTORPOD(const MessagePack &);
        void on_MPK_BINDCHANNEL(const MessagePack &);
        void on_MPK_CHECKMASTER(const MessagePack &);
        void on_MPK_SHOWDROPITEM(const MessagePack &);
        void on_MPK_NPCXMLLAYOUT(const MessagePack &);
        void on_MPK_QUERYCORECORD(const MessagePack &);
        void on_MPK_QUERYLOCATION(const MessagePack &);
        void on_MPK_REMOVEGROUNDITEM(const MessagePack &);

    private:
        void net_CM_REQUESTKILLPETS   (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTSPACEMOVE  (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTMAGICDAMAGE(uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYCORECORD     (uint8_t, const uint8_t *, size_t);
        void net_CM_ACTION            (uint8_t, const uint8_t *, size_t);
        void net_CM_PICKUP            (uint8_t, const uint8_t *, size_t);
        void net_CM_PING              (uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYGOLD         (uint8_t, const uint8_t *, size_t);
        void net_CM_NPCEVENT          (uint8_t, const uint8_t *, size_t);

    protected:
        void reportGold();
        void reportStand();
        void reportHealth();
        void reportDeadUID(uint64_t);
        void reportCO(uint64_t);
        void reportOffline(uint64_t, uint32_t);

    protected:
        virtual void reportAction(uint64_t, const ActionNode &);

    protected:
        void dispatchOffline();

    protected:
        bool StruckDamage(const DamageNode &);

    protected:
        void RequestKillPets();

    protected:
        DamageNode GetAttackDamage(int);

    protected:
        bool DCValid(int, bool);
        bool InRange(int, int, int);

    protected:
        bool ActionValid(const ActionNode &);
        bool MotionValid(const ActionNode &);

    protected:
        void onCMActionMove  (CMAction);
        void onCMActionStand (CMAction);
        void onCMActionSpell (CMAction);
        void onCMActionAttack(CMAction);
        void onCMActionPickUp(CMAction);

    private:
        bool postNetMessage(uint8_t, const void *, size_t);
        template<typename T> bool postNetMessage(uint8_t nHC, T& stMessage)
        {
            return postNetMessage(nHC, (const uint8_t *)(&stMessage), sizeof(stMessage));
        }

    protected:
        virtual bool goDie();
        virtual bool goGhost();

    protected:
        bool Offline();

    protected:
        virtual int MaxStep() const;

    protected:
        virtual void RecoverHealth();

    protected:
        uint32_t GetLevelExp();

    protected:
        size_t dbUpdate(const char *, const char *, ...);
        size_t dbAccess(const char *, const char *, std::function<std::string(std::string)>);

    protected:
        void GainExp(int);

    protected:
        void PullRectCO(int, int);

    protected:
        bool CanPickUp(uint32_t, uint32_t);

    protected:
        bool DBLoadPlayer();
        bool DBSavePlayer();

    protected:
        void checkFriend(uint64_t, std::function<void(int)>) override;
};

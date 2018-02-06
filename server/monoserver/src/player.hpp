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
#include <cstdint>
#include "monoserver.hpp"
#include "charobject.hpp"

class Player final: public CharObject
{
    protected:
        const uint32_t m_DBID;
        const uint32_t m_JobID;

    protected:
        uint32_t m_ChannID;

    protected:
        uint32_t m_Exp;
        uint32_t m_Level;

    protected:
        uint32_t m_Gold;
        std::vector<CommonItem> m_Inventory;

    public:
        Player(uint32_t,                // DBID 
                ServiceCore *,          //
                ServerMap *,            //
                int,                    // map x
                int,                    // map y
                int,                    // direction
                uint8_t);               // life cycle state

    public:
        ~Player();

    protected:
        uint32_t Exp() const
        {
            return m_Exp;
        }

        uint32_t Gold() const
        {
            return m_Gold;
        }

        uint32_t Level() const
        {
            return m_Level;
        }

        uint32_t DBID() const
        {
            return m_DBID;
        }

        uint32_t JobID() const
        {
            return m_JobID;
        }

        uint32_t ChannID()
        {
            return m_ChannID;
        }

    public:
        int Speed()
        {
            return 5;
        }

        bool Update();

    public:
        InvarData GetInvarData() const;

    protected:
        void OperateNet(uint8_t, const uint8_t *, size_t);

    protected:
        void OperateAM(const MessagePack &, const Theron::Address &);

    private:
        void On_MPK_EXP(const MessagePack &, const Theron::Address &);
        void On_MPK_ACTION(const MessagePack &, const Theron::Address &);
        void On_MPK_ATTACK(const MessagePack &, const Theron::Address &);
        void On_MPK_OFFLINE(const MessagePack &, const Theron::Address &);
        void On_MPK_CORECORD(const MessagePack &, const Theron::Address &);
        void On_MPK_PICKUPOK(const MessagePack &, const Theron::Address &);
        void On_MPK_UPDATEHP(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_MAPSWITCH(const MessagePack &, const Theron::Address &);
        void On_MPK_NETPACKAGE(const MessagePack &, const Theron::Address &);
        void On_MPK_BADCHANNEL(const MessagePack &, const Theron::Address &);
        void On_MPK_NOTIFYDEAD(const MessagePack &, const Theron::Address &);
        void On_MPK_NOTIFYNEWCO(const MessagePack &, const Theron::Address &);
        void On_MPK_DEADFADEOUT(const MessagePack &, const Theron::Address &);
        void On_MPK_BINDCHANNEL(const MessagePack &, const Theron::Address &);
        void On_MPK_SHOWDROPITEM(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYCORECORD(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYLOCATION(const MessagePack &, const Theron::Address &);
        void On_MPK_REMOVEGROUNDITEM(const MessagePack &, const Theron::Address &);

    private:
        void Net_CM_REQUESTSPACEMOVE(uint8_t, const uint8_t *, size_t);
        void Net_CM_QUERYCORECORD   (uint8_t, const uint8_t *, size_t);
        void Net_CM_ACTION          (uint8_t, const uint8_t *, size_t);
        void Net_CM_PICKUP          (uint8_t, const uint8_t *, size_t);
        void Net_CM_QUERYGOLD       (uint8_t, const uint8_t *, size_t);

    private:
        void For_CheckTime();

    protected:
        void ReportGold();
        void ReportStand();
        void ReportHealth();
        void ReportCORecord(uint32_t);
        void ReportOffline(uint32_t, uint32_t);
        void ReportAction(uint32_t, const ActionNode &);

    protected:
        void DispatchOffline();

    protected:
        bool StruckDamage(const DamageNode &);

    protected:
        DamageNode GetAttackDamage(int);

    protected:
        bool DCValid(int, bool);
        bool InRange(int, int, int);

    protected:
        bool ActionValid(const ActionNode &);
        bool MotionValid(const ActionNode &);

    protected:
        void CheckFriend(uint32_t, const std::function<void(int)> &);

    protected:
        void OnCMActionMove  (CMAction);
        void OnCMActionStand (CMAction);
        void OnCMActionSpell (CMAction);
        void OnCMActionAttack(CMAction);
        void OnCMActionPickUp(CMAction);

    private:
        bool PostNetMessage(uint8_t, const uint8_t *, size_t);
        template<typename T> bool PostNetMessage(uint8_t nHC, T& stMessage)
        {
            return PostNetMessage(nHC, (const uint8_t *)(&stMessage), sizeof(stMessage));
        }

    protected:
        virtual bool GoDie();
        virtual bool GoGhost();
        virtual bool GoSuicide();

    protected:
        bool Offline();

    protected:
        virtual int MaxStep();

    protected:
        virtual void RecoverHealth();

    protected:
        uint32_t GetLevelExp();

    protected:
        bool DBUpdate(const char *, const char *, ...);
        bool DBAccess(const char *, const char *, std::function<std::string(const char *)>);

    protected:
        void GainExp(int);

    protected:
        void PullRectCO(int, int);

    protected:
        bool CanPickUp(uint32_t, uint32_t);

    protected:
        bool DBLoadPlayer();
        bool DBSavePlayer();
};

/*
 * =====================================================================================
 *
 *       Filename: player.hpp
 *        Created: 04/08/2016 22:37:01
 *  Last Modified: 01/25/2018 12:53:18
 *
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

#pragma pack(push, 1)
typedef struct stPLAYERFEATURE
{
    uint8_t     Gender;
    uint8_t     Wear;
    uint8_t     Hair;
    uint8_t     Weapon;

    stPLAYERFEATURE()
    {
        std::memset(this, 0, sizeof(*this));
    }
}PLAYERFEATURE;

typedef struct stPLAYERFEATUREEX
{
    uint8_t     Horse;
    uint32_t    HairColor;
    uint32_t    WearColor;

    stPLAYERFEATUREEX()
    {
        std::memset(this, 0, sizeof(*this));
    }
}PLAYERFEATUREEX;
#pragma pack(pop)

class Player final: public CharObject
{
    protected:
        const uint32_t m_DBID;
        const uint32_t m_JobID;

    protected:
        uint32_t m_SessionID;
        uint32_t m_Level;

    protected:
        uint32_t m_Gold;
        std::vector<CommonItem> m_Inventory;

    protected:
        PLAYERFEATURE   m_Feature;
        PLAYERFEATUREEX m_FeatureEx;

    public:
        Player(uint32_t,                // GUID
                ServiceCore *,          //
                ServerMap *,            //
                int,                    // map x
                int,                    // map y
                int,                    // direction
                uint8_t);               // life cycle state

    public:
        ~Player();

    public:
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

        uint32_t SessionID()
        {
            return m_SessionID;
        }

    public:
        int Speed()
        {
            return 5;
        }

        bool Update();

        bool Bind(uint32_t);

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
        void On_MPK_BADSESSION(const MessagePack &, const Theron::Address &);
        void On_MPK_NOTIFYDEAD(const MessagePack &, const Theron::Address &);
        void On_MPK_DEADFADEOUT(const MessagePack &, const Theron::Address &);
        void On_MPK_BINDSESSION(const MessagePack &, const Theron::Address &);
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
        int GetLevelExp();

    protected:
        bool DBRecord(const char *, std::function<std::string(const char *)>);
        bool DBRecord(const char *, std::function<const char *(const char *, char *, size_t)>);

    protected:
        void DBRecordLevelUp();
        void DBRecordGainExp(int);

    protected:
        void PullRectCO(int, int);

    protected:
        bool CanPickUp(uint32_t, uint32_t);

    protected:
        bool DBCommand(const char *, const char * = nullptr);

    protected:
        bool DBLoadPlayer();
        bool DBSavePlayer();
};

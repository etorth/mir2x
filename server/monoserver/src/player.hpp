/*
 * =====================================================================================
 *
 *       Filename: player.hpp
 *        Created: 04/08/2016 22:37:01
 *  Last Modified: 05/16/2017 23:05:23
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

class Player: public CharObject
{
    protected:
        const uint32_t m_DBID;
        const uint32_t m_JobID;

    protected:
        uint32_t m_SessionID;
        uint32_t m_Level;

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
       ~Player() = default;

    public:
        uint32_t DBID() const
        {
            return m_DBID;
        }

        uint32_t JobID() const
        {
            return m_JobID;
        }

    public:
        int Speed()
        {
            return 5;
        }

        bool Update();

        bool Bind(uint32_t);

    protected:
        void OperateNet(uint8_t, const uint8_t *, size_t);

    protected:
        void Operate(const MessagePack &, const Theron::Address &);

    private:
        void On_MPK_NOTICE(const MessagePack &, const Theron::Address &);
        void On_MPK_ACTION(const MessagePack &, const Theron::Address &);
        void On_MPK_ATTACK(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_MAPSWITCH(const MessagePack &, const Theron::Address &);
        void On_MPK_NETPACKAGE(const MessagePack &, const Theron::Address &);
        void On_MPK_PULLCOINFO(const MessagePack &, const Theron::Address &);
        void On_MPK_BINDSESSION(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYLOCATION(const MessagePack &, const Theron::Address &);

    private:
        void Net_CM_QUERYMONSTERGINFO(uint8_t, const uint8_t *, size_t);
        void Net_CM_ACTION           (uint8_t, const uint8_t *, size_t);

    private:
        void For_CheckTime();

    protected:
        void ReportStand();
        void ReportCORecord(uint32_t);

    protected:
        int GetAttackPower(int);

    protected:
        bool InRange(int, int, int);
};

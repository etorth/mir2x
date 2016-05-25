/*
 * =====================================================================================
 *
 *       Filename: player.hpp
 *        Created: 04/08/2016 22:37:01
 *  Last Modified: 05/24/2016 21:42:14
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
typedef struct stPLAYERFEATURE{
    uint8_t     Gender;
    uint8_t     Wear;
    uint8_t     Hair;
    uint8_t     Weapon;

    stPLAYERFEATURE()
    {
        std::memset(this, 0, sizeof(*this));
    }
}PLAYERFEATURE;

typedef struct stPLAYERFEATUREEX{
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
        uint32_t m_GUID;
        uint32_t m_JobID;

    protected:
        PLAYERFEATURE   m_Feature;
        PLAYERFEATUREEX m_FeatureEx;

    public:
        Player(uint32_t, uint32_t);
        ~Player();

    public:
        // type test function
        virtual bool Type(uint8_t);
        virtual bool State(uint8_t);

        virtual bool ResetType(uint8_t, bool);
        virtual bool ResetState(uint8_t, bool);

        virtual uint32_t NameColor();
        virtual const char *CharName();

        virtual int Range(uint8_t);

    public:
        int Speed()
        {
            return 5;
        }

        bool Update();

    protected:
        void OperateNet(uint8_t, const uint8_t *, size_t);

    protected:
        void Operate(const MessagePack &, const Theron::Address &);

    public:
        static void OnReadHC(uint8_t, Session *);

    private:
        void On_MPK_NETPACKAGE(const MessagePack &, const Theron::Address &);

    private:
        void Net_CM_MOTION(uint8_t, const uint8_t *, size_t);

    private:
        void For_CheckTime();
};

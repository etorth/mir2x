/*
 * =====================================================================================
 *
 *       Filename: monsterrecord.hpp
 *        Created: 05/08/2017 16:21:14
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
#include <array>
#include <cstdint>
#include "protocoldef.hpp"

class MonsterRecord
{
    public:
        const char8_t *name;

        int level;
        int undead;
        int tamable;
        int coolEye;

        int lookID;

        int HP;
        int MP;
        int hit;
        int exp;

        int DC;
        int DCMax;
        int MDC;
        int MDCMax;
        int AC;
        int ACMax;
        int MAC;
        int MACMax;

        int ACFire;
        int ACIce;
        int ACLight;
        int ACWind;
        int ACHoly;
        int ACDark;
        int ACPhantom;

        int walkWait;
        int walkSpeed;

        int attackMode;
        int attackWait;
        int attackEffect;

        int DCType0;
        int DCType1;
        int DCType2;
        int DCType3;
        int DCType4;
        int DCType5;
        int DCType6;
        int DCType7;

    public:
        constexpr MonsterRecord(
                const char8_t *argName,

                int argLevel,
                int argUndead,
                int argTamable,
                int argCoolEye,

                int argLookID,

                int argHP,
                int argMP,
                int argHit,
                int argExp,

                int argDC,
                int argDCMax,
                int argMDC,
                int argMDCMax,
                int argAC,
                int argACMax,
                int argMAC,
                int argMACMax,

                int argACFire,
                int argACIce,
                int argACLight,
                int argACWind,
                int argACHoly,
                int argACDark,
                int argACPhantom,

                int argWalkWait,
                int argWalkSpeed,

                int argAttackMode,
                int argAttackWait,
                int argAttackEffect,

                const char8_t *argDC0,
                const char8_t *argDC1,
                const char8_t *argDC2,
                const char8_t *argDC3,
                const char8_t *argDC4,
                const char8_t *argDC5,
                const char8_t *argDC6,
                const char8_t *argDC7)
            : name(argName ? argName : u8"")
            , level(argLevel)
            , undead(argUndead)
            , tamable(argTamable)
            , coolEye(argCoolEye)

            , lookID(argLookID)

            , HP(argHP)
            , MP(argMP)
            , hit(argHit)
            , exp(argExp)

            , DC(argDC)
            , DCMax(argDCMax)
            , MDC(argMDC)
            , MDCMax(argMDCMax)
            , AC(argAC)
            , ACMax(argACMax)
            , MAC(argMAC)
            , MACMax(argMACMax)

            , ACFire(argACFire)
            , ACIce(argACIce)
            , ACLight(argACLight)
            , ACWind(argACWind)
            , ACHoly(argACHoly)
            , ACDark(argACDark)
            , ACPhantom(argACPhantom)

            , walkWait(argWalkWait)
            , walkSpeed(argWalkSpeed)

            , attackMode(argAttackMode)
            , attackWait(argAttackWait)
            , attackEffect(argAttackEffect)

            , DCType0(_inn_MRDCType(argDC0))
            , DCType1(_inn_MRDCType(argDC1))
            , DCType2(_inn_MRDCType(argDC2))
            , DCType3(_inn_MRDCType(argDC3))
            , DCType4(_inn_MRDCType(argDC4))
            , DCType5(_inn_MRDCType(argDC5))
            , DCType6(_inn_MRDCType(argDC6))
            , DCType7(_inn_MRDCType(argDC7))
        {}

    private:
        static constexpr int _inn_MRDCType(const char8_t *argDCName)
        {
            using namespace std::literals;
            if(argDCName){
                if(u8"普通攻击"sv == argDCName) return DC_PHY_PLAIN;
                else                            return DC_NONE;
            }
            return DC_NONE;
        }

    public:
        operator bool() const
        {
            return name[0] != '\0';
        }

        std::array<int, 8> DCList() const
        {
            return {{DCType0, DCType1, DCType2, DCType3, DCType4, DCType5, DCType6, DCType7}};
        }
};

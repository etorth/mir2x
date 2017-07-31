/*
 * =====================================================================================
 *
 *       Filename: monsterrecord.hpp
 *        Created: 05/08/2017 16:21:14
 *  Last Modified: 07/30/2017 15:40:32
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
#include <array>
#include <cstdint>
#include "protocoldef.hpp"

class MonsterRecord
{
    public:
        const char *Name;

        int Level;
        int Undead;
        int Tameable;
        int CoolEye;

        int HP;
        int MP;
        int Hit;
        int Exp;

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

        int WalkWait;
        int WalkSpeed;

        int AttackMode;
        int AttackWait;
        int AttackEffect;

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
                const char *szName,

                int nLevel,
                int nUndead,
                int nTameable,
                int nCoolEye,

                int nHP,
                int nMP,
                int nHit,
                int nExp,

                int nDC,
                int nDCMax,
                int nMDC,
                int nMDCMax,
                int nAC,
                int nACMax,
                int nMAC,
                int nMACMax,

                int nACFire,
                int nACIce,
                int nACLight,
                int nACWind,
                int nACHoly,
                int nACDark,
                int nACPhantom,

                int nWalkWait,
                int nWalkSpeed,

                int nAttackMode,
                int nAttackWait,
                int nAttackEffect,

                const char *szDC0,
                const char *szDC1,
                const char *szDC2,
                const char *szDC3,
                const char *szDC4,
                const char *szDC5,
                const char *szDC6,
                const char *szDC7)
            : Name(szName ? szName : "")
            , Level(nLevel)
            , Undead(nUndead)
            , Tameable(nTameable)
            , CoolEye(nCoolEye)

            , HP(nHP)
            , MP(nMP)
            , Hit(nHit)
            , Exp(nExp)

            , DC(nDC)
            , DCMax(nDCMax)
            , MDC(nMDC)
            , MDCMax(nMDCMax)
            , AC(nAC)
            , ACMax(nACMax)
            , MAC(nMAC)
            , MACMax(nMACMax)

            , ACFire(nACFire)
            , ACIce(nACIce)
            , ACLight(nACLight)
            , ACWind(nACWind)
            , ACHoly(nACHoly)
            , ACDark(nACDark)
            , ACPhantom(nACPhantom)

            , WalkWait(nWalkWait)
            , WalkSpeed(nWalkSpeed)

            , AttackMode(nAttackMode)
            , AttackWait(nAttackWait)
            , AttackEffect(nAttackEffect)

            , DCType0(_Inn_MRDCType(szDC0))
            , DCType1(_Inn_MRDCType(szDC1))
            , DCType2(_Inn_MRDCType(szDC2))
            , DCType3(_Inn_MRDCType(szDC3))
            , DCType4(_Inn_MRDCType(szDC4))
            , DCType5(_Inn_MRDCType(szDC5))
            , DCType6(_Inn_MRDCType(szDC6))
            , DCType7(_Inn_MRDCType(szDC7))
        {}

    private:
        // need define some internal functions for constructor
        // c++14 currently doesn't not support constexpr lambda
        static constexpr bool _Inn_CompareUTF8(const char *szStr1, const char *szStr2)
        {
            if(szStr1 && szStr2){
                while(*szStr1 == *szStr2){
                    if(*szStr1){
                        ++szStr1;
                        ++szStr2;
                    }else{
                        return true;
                    }
                }
            }
            return false;
        }

        static constexpr int _Inn_MRDCType(const char *szDCName)
        {
            if(szDCName){
                if(false){
                }else if(_Inn_CompareUTF8(szDCName, u8"普通攻击")){ return DC_PHY_PLAIN;
                }else                                               return DC_NONE;
            }
            return DC_NONE;
        }

    public:
        operator bool() const
        {
            return Name[0] != '\0';
        }

        std::array<int, 8> DCList() const
        {
            return {DCType0, DCType1, DCType2, DCType3, DCType4, DCType5, DCType6, DCType7};
        }
};

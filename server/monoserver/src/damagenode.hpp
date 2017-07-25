/*
 * =====================================================================================
 *
 *       Filename: damagenode.hpp
 *        Created: 07/21/2017 17:12:19
 *  Last Modified: 07/24/2017 22:55:18
 *
 *    Description: description of a damage
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
#include <utility>
#include <cstdint>
#include <cstdlib>
#include <cinttypes>
#include "protocoldef.hpp"

class DamageNode
{
    private:
        // use std::array but which is aggregrate
        // EffectArrayType need to support initialized by (pBuf, nBufSize)
        class EffectArrayType
        {
            private:
                std::array<int, 32> Array;

            public:
                template<typename... U> EffectArrayType(EffectType nEffect, U&&... u)
                    : Array {{nEffect, std::forward<U>(u)...}}
                {
                    CheckEffect();
                }

                EffectArrayType(const int *pBuf = nullptr, size_t nBufSize = 0)
                    : Array()
                {
                    if(pBuf){
                        if(nBufSize == 0){
                            while(pBuf[nBufSize++] != EFF_NONE){
                                continue;
                            }
                        }

                        for(size_t nIndex = 0; nIndex < nBufSize; ++nIndex){
                            if(pBuf[nIndex] != EFF_NONE){
                                Array[nIndex] = pBuf[nIndex];
                            }else{
                                LogError(2, "Invalid effect array");
                            }
                        }
                    }else{
                        if(nBufSize != 0){
                            LogError(2, "Invalid effect array length");
                        }
                    }
                }

            public:
                const int *Data() const
                {
                    return &(Array[0]);
                }

                // error-prone:
                // this function returns record size, not byte count
                size_t DataLen() const
                {
                    return Array.size();
                }

            private:
                void LogError(int, const char *) const;
                void CheckEffect() const;
        };

    public:
        const uint32_t UID;

        const int Type;
        const int Damage;
        const int Element;

        const EffectArrayType EffectArray;

    public:
        template<typename... U> DamageNode(uint32_t nUID, int nType, int nDamage, int nElement, U&&... u)
            : UID(nUID)
            , Type(nType)
            , Damage(nDamage)
            , Element(nElement)
            , EffectArray{std::forward<U>(u)...}
        {}

        DamageNode()
            : DamageNode(0, 0, 0, 0)
        {}

    public:
        void Print() const;

        // return Effect param, or -1 if not found
        // param: Effect : Effect % 256 == 0 : find effect class and return param
        //                 Effect % 256 != 0 : find effect value and return param
        int EffectParam(int) const;

    public:
        operator bool () const
        {
            return true
                && UID  != 0            // damage should come from s.b.
                && Type != DC_NONE;     //
        }
};

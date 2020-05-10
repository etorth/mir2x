/*
 * =====================================================================================
 *
 *       Filename: damagenode.hpp
 *        Created: 07/21/2017 17:12:19
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
                std::array<int, 32> m_effect;

            private:
                size_t m_effectLen;

            public:
                template<typename... U> EffectArrayType(int nEffect, U&&... u)
                    : m_effect{{nEffect, std::forward<U>(u)...}}
                    , m_effectLen(sizeof...(u) + 1)
                {
                    // for aggregrate initialization
                    // we need to make sure the provided effect list is valid
                    // then to count for the effect length

                    for(size_t nIndex = 0; nIndex < sizeof...(u) + 1; ++nIndex){
                        if(m_effect[nIndex] == EFF_NONE){
                            LogError(2, "Invalid effect(s) provided");
                        }
                    }
                }

                EffectArrayType(const int *pBuf = nullptr, size_t nBufSize = 0)
                    : m_effect()
                    , m_effectLen(0)
                {
                    if(pBuf){
                        if(nBufSize == 0){
                            while(pBuf[nBufSize] != EFF_NONE){
                                nBufSize++;
                            }
                        }

                        m_effectLen = nBufSize;
                        for(size_t nIndex = 0; nIndex < nBufSize; ++nIndex){
                            if(pBuf[nIndex] != EFF_NONE){
                                m_effect[nIndex] = pBuf[nIndex];
                            }else{
                                LogError(2, "Invalid effect(s) provided");
                            }
                        }
                    }else{
                        m_effectLen = 0;
                        if(nBufSize){
                            LogError(2, "Invalid effect array length");
                        }
                    }
                }

            public:
                const int *Effect() const
                {
                    // even m_effectLen is zero
                    // we should always return a valid pointer

                    // because we support:
                    // EffectArray(stEffectArray.Effect())

                    return &(m_effect[0]);
                }

                // error-prone:
                // this function returns record size, not byte count
                size_t EffectLen() const
                {
                    return m_effectLen;
                }

            private:
                void LogError(int, const char *) const;
        };

    public:
        const uint64_t UID;

        const int Type;
        const int Damage;
        const int Element;

        const EffectArrayType EffectArray;

    public:
        template<typename... U> DamageNode(uint64_t nUID, int nType, int nDamage, int nElement, U&&... u)
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

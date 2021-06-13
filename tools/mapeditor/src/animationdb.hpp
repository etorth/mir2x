/*
 * =====================================================================================
 *
 *       Filename: animationdb.hpp
 *        Created: 06/22/2016 18:19:55
 *    Description: db for testing animation
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
#include <string>
#include "animation.hpp"

class AnimationDB
{
    private:
        std::vector<Animation>  m_animationV;
        Animation               m_emptyRecord;
        std::string             m_dbPath;

    public:
        AnimationDB()
            : m_animationV()
            , m_emptyRecord()
            , m_dbPath("")
        {}
        ~AnimationDB() = default;

    public:
        // TODO: we return non-const ref here since we need to update it
        Animation &RetrieveAnimation(uint32_t nMonsterID)
        {
            for(auto &rstRecord: m_animationV){
                if(rstRecord.MonsterID() == nMonsterID){
                    return rstRecord;
                }
            }

            return m_emptyRecord;
        }

        size_t Size()
        {
            return m_animationV.size();
        }

        size_t Count()
        {
            return Size();
        }

        // TODO: when using this function, we need to Size() function
        Animation &Get(size_t nVID)
        {
            return m_animationV[nVID];
        }

        template<typename... T> bool Add(uint32_t nMonsterID, uint32_t nAction, uint32_t nDirection, uint32_t nFrame, bool bShadow, T... stT)
        {
            if(false
                    || nMonsterID ==  0
                    || nAction    >= 16
                    || nDirection >=  8
                    || nFrame     >= 32){ return false; }

            for(auto &rstRecord: m_animationV){
                if(rstRecord.MonsterID() == nMonsterID){
                    return rstRecord.Add(nAction, nDirection, nFrame, bShadow, std::forward<T>(stT)...);
                }
            }

            m_animationV.emplace_back(nMonsterID);
            return m_animationV.back().Add(nAction, nDirection, nFrame, bShadow, std::forward<T>(stT)...);
        }
    public:
        bool Load(const char *);
};

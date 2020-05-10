/*
 * =====================================================================================
 *
 *       Filename: magicbase.hpp
 *        Created: 08/05/2017 22:58:20
 *    Description: base of AttachMagic and IndepMagic
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
#include <list>
#include <functional>
#include "magicrecord.hpp"

class MagicBase
{
    protected:
        int m_ID;
        int m_param;
        int m_stage;

    protected:
        double m_timeOut;
        double m_accuTime;

    protected:
        // mutable means already thread safe
        // don't use this class in multithread env since not atomic protection
        mutable const GfxEntry *m_cacheEntry;

    protected:
        std::list<std::function<bool()>> m_updateFunc;

    public:
        MagicBase(int,      // MagicID
                int,        // MagicParam
                int,        // MagicStage
                double);    // TimeOut

        MagicBase(int,      // MagicID
                int,        // MagicParam
                int);       // MagicStage

    public:
        virtual ~MagicBase() = default;

    public:
        int ID() const
        {
            return m_ID;
        }

        int Param() const
        {
            return m_param;
        }

        int Stage() const
        {
            return m_stage;
        }

    public:
        int Frame() const;

    public:
        // hard to make an unformed version
        // different magic we need different method to check done
        virtual bool Done() const = 0;

    public:
        virtual void Draw(int, int) = 0;
        virtual void Update(double) = 0;

    public:
        void AddFunc(std::function<bool()> fnOnUpdate)
        {
            m_updateFunc.push_back(fnOnUpdate);
        }

    protected:
        void ExecUpdateFunc()
        {
            for(auto p = m_updateFunc.begin(); p != m_updateFunc.end();){
                if((*p)()){
                    p = m_updateFunc.erase(p);
                }else{
                    ++p;
                }
            }
        }

    public:
        bool StageDone() const;

    public:
        operator bool () const
        {
            // RefreshCache() makes m_cacheEntry point to correct entry
            // if current magic configuration is invalid it fails

            return RefreshCache();
        }

        void Print() const;

    protected:
        bool RefreshCache() const;
};

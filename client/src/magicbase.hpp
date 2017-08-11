/*
 * =====================================================================================
 *
 *       Filename: magicbase.hpp
 *        Created: 08/05/2017 22:58:20
 *  Last Modified: 08/10/2017 16:56:01
 *
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
#include "magicrecord.hpp"

class MagicBase
{
    protected:
        int m_ID;
        int m_Param;
        int m_Stage;

    protected:
        double m_TimeOut;
        double m_AccuTime;

    protected:
        // mutable means already thread safe
        // don't use this class in multithread env since not atomic protection
        mutable const GfxEntry *m_CacheEntry;

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
            return m_Param;
        }

        int Stage() const
        {
            return m_Stage;
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
        bool StageDone() const;

    public:
        operator bool () const
        {
            // RefreshCache() makes m_CacheEntry point to correct entry
            // if current magic configuration is invalid it fails

            return RefreshCache();
        }

        void Print() const;

    protected:
        bool RefreshCache() const;
};

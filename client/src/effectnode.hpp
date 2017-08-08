/*
 * =====================================================================================
 *
 *       Filename: effectnode.hpp
 *        Created: 08/05/2017 22:58:20
 *  Last Modified: 08/08/2017 16:52:29
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
#include "dbcomrecord.hpp"
#include "magicrecord.hpp"
class EffectNode
{
    private:
        int m_MagicID;
        int m_MagicParam;
        int m_GfxEntryID;

    private:
        double m_TimeOut;
        double m_AccuTime;

    public:
        EffectNode(int,     // MagicID
                int,        // MagicParam
                int,        // GfxEntryID
                double);    // TimeOut

        EffectNode(int,     // MagicID
                int,        // MagicParam
                int);       // GfxEntryID

    public:
        int MagicID() const
        {
            return m_MagicID;
        }

        int MagicParam() const
        {
            return m_MagicParam;
        }

        int GfxEntryID() const
        {
            return m_GfxEntryID;
        }

    public:
        int Frame() const;

    public:
        void Update(double);

    public:
        void Draw(int, int);

    public:
        bool Done() const
        {
            if(auto &rstGfxEntry = DBCOM_MAGICRECORD(MagicID()).GetGfxEntry(GfxEntryID())){
                if(false
                        || rstGfxEntry.Loop == 1
                        || Frame() < rstGfxEntry.FrameCount - 1){
                    return false;
                }
            }
            return true;
        }

    public:
        operator bool () const
        {
            return (MagicID() > 0) && (GfxEntryID() >= 0);
        }

        void Print() const
        {
        }
};

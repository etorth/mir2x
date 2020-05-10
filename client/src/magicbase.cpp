/*
 * =====================================================================================
 *
 *       Filename: magicbase.cpp
 *        Created: 08/08/2017 12:43:18
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

#include "log.hpp"
#include "sdldevice.hpp"
#include "magicbase.hpp"
#include "dbcomrecord.hpp"
#include "pngtexoffdb.hpp"

extern Log *g_log;

MagicBase::MagicBase(int nMagicID,
        int nMagicParam,
        int nMagicStage,
        double fTimeOut)
    : m_ID(nMagicID)
    , m_param(nMagicParam)
    , m_stage(nMagicStage)
    , m_timeOut(fTimeOut)
    , m_accuTime(0.0)
    , m_cacheEntry(nullptr)
{
    if(!RefreshCache()){
        throw fflerror("invalid argument to MagicBase");
    }
}

MagicBase::MagicBase(int nMagicID, int nMagicParam, int nMagicStage)
    : MagicBase(nMagicID, nMagicParam, nMagicStage, -1.0)
{}

int MagicBase::Frame() const
{
    if(RefreshCache()){
        int nRealFrame = (m_accuTime / 1000.0) * SYS_DEFFPS * (m_cacheEntry->Speed / 100.0);
        switch(m_cacheEntry->Loop){
            case 0:
                {
                    return nRealFrame;
                }
            case 1:
                {
                    return nRealFrame % m_cacheEntry->FrameCount;
                }
            default:
                {
                    break;
                }
        }
    }
    return -1;
}

bool MagicBase::StageDone() const
{
    if(!RefreshCache()){
        return true;
    }

    // 1. loop effect
    // 2. not looping but not finished yet

    if(m_cacheEntry->Loop){
        return false;
    }

    // I allow the last frame as ``incomplete"
    // otherwise I can't show the last magic frame in Draw()
    if(Frame() <= (m_cacheEntry->FrameCount - 1)){
        return false;
    }

    return true;
}

bool MagicBase::RefreshCache() const
{
    if(m_cacheEntry && (m_cacheEntry->Stage == Stage())){
        return true;
    }

    // need to update it
    // current cache entry doesn't match / is not valid

    if(auto &rstMR = DBCOM_MAGICRECORD(ID())){
        for(int nGfxEntryIndex = 0;; ++nGfxEntryIndex){
            if(auto &rstGfxEntry = rstMR.GetGfxEntry(nGfxEntryIndex)){
                if(rstGfxEntry.Stage == Stage()){
                    m_cacheEntry = &rstGfxEntry;
                    return true;
                }
            }else{
                break;
            }
        }
    }

    m_cacheEntry = nullptr;
    return false;
}

void MagicBase::Print() const
{
}

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
#include "pngtexoffdbn.hpp"

MagicBase::MagicBase(int nMagicID,
        int nMagicParam,
        int nMagicStage,
        double fTimeOut)
    : m_ID(nMagicID)
    , m_Param(nMagicParam)
    , m_Stage(nMagicStage)
    , m_TimeOut(fTimeOut)
    , m_AccuTime(0.0)
    , m_CacheEntry(nullptr)
{
    if(!RefreshCache()){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Invalid argument to MagicBase");
        Print();
    }
}

MagicBase::MagicBase(int nMagicID, int nMagicParam, int nMagicStage)
    : MagicBase(nMagicID, nMagicParam, nMagicStage, -1.0)
{}

int MagicBase::Frame() const
{
    if(RefreshCache()){
        int nRealFrame = (m_AccuTime / 1000.0) * SYS_DEFFPS * (m_CacheEntry->Speed / 100.0);
        switch(m_CacheEntry->Loop){
            case 0:
                {
                    return nRealFrame;
                }
            case 1:
                {
                    return nRealFrame % m_CacheEntry->FrameCount;
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

    if(m_CacheEntry->Loop){
        return false;
    }

    // I allow the last frame as ``incomplete"
    // otherwise I can't show the last magic frame in Draw()
    if(Frame() <= (m_CacheEntry->FrameCount - 1)){
        return false;
    }

    return true;
}

bool MagicBase::RefreshCache() const
{
    if(m_CacheEntry && (m_CacheEntry->Stage == Stage())){
        return true;
    }

    // need to update it
    // current cache entry doesn't match / is not valid

    if(auto &rstMR = DBCOM_MAGICRECORD(ID())){
        for(int nGfxEntryIndex = 0;; ++nGfxEntryIndex){
            if(auto &rstGfxEntry = rstMR.GetGfxEntry(nGfxEntryIndex)){
                if(rstGfxEntry.Stage == Stage()){
                    m_CacheEntry = &rstGfxEntry;
                    return true;
                }
            }else{
                break;
            }
        }
    }

    m_CacheEntry = nullptr;
    return false;
}

void MagicBase::Print() const
{
}

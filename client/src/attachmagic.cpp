/*
 * =====================================================================================
 *
 *       Filename: attachmagic.cpp
 *        Created: 08/10/2017 12:46:45
 *  Last Modified: 08/10/2017 17:08:16
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
#include "log.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "dbcomrecord.hpp"
#include "attachmagic.hpp"
#include "pngtexoffdbn.hpp"

AttachMagic::AttachMagic(int nMagicID, int nMagicParam, int nMagicStage, double fLastTime)
    : MagicBase(nMagicID, nMagicParam, nMagicStage, fLastTime)
{
    if(RefreshCache()){
        switch(m_CacheEntry->Type){
            case EGT_BOUND:
                {
                    break;
                }
            default:
                {
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_FATAL, "Invalid GfxEntry::Type to AttachMagic");
                    m_CacheEntry->Print();
                    break;
                }
        }
    }
}

AttachMagic::AttachMagic(int nMagicID, int nMagicParam, int nMagicStage)
    : AttachMagic(nMagicID, nMagicParam, nMagicStage, -1.0)
{}

void AttachMagic::Update(double fTime)
{
    if(!Done()){
        m_AccuTime += fTime;

        if(StageDone()){
            auto fnCheckStageValid = [this](int nNewStage) -> bool
            {
                if(auto &rstMR = DBCOM_MAGICRECORD(ID())){
                    for(int nGfxEntryIndex = 0;; ++nGfxEntryIndex){
                        if(auto &rstGfxEntry = rstMR.GetGfxEntry(nGfxEntryIndex)){
                            if(rstGfxEntry.Stage == nNewStage){
                                return true;
                            }
                        }else{ break; }
                    }
                }
                return false;
            };

            // find next valid stage
            // if no stage valid set EGS_NONE
            // MagicBase::Done() returns true for EGS_NONE

            switch(Stage()){
                case EGS_INIT:
                    {
                        if(fnCheckStageValid(EGS_RUN)){
                            m_Stage = EGS_RUN;
                        }else if(fnCheckStageValid(EGS_DONE)){
                            m_Stage = EGS_DONE;
                        }else{
                            m_Stage = EGS_NONE;
                        }
                        break;
                    }
                case EGS_START:
                    {
                        if(fnCheckStageValid(EGS_RUN)){
                            m_Stage = EGS_RUN;
                        }else if(fnCheckStageValid(EGS_DONE)){
                            m_Stage = EGS_DONE;
                        }else{
                            m_Stage = EGS_NONE;
                        }
                        break;
                    }
                case EGS_RUN:
                    {
                        if(fnCheckStageValid(EGS_DONE)){
                            m_Stage = EGS_DONE;
                        }else{
                            m_Stage = EGS_NONE;
                        }
                        break;
                    }
                case EGS_DONE:
                    {
                        m_Stage = EGS_NONE;
                        break;
                    }
                default:
                    {
                        break;
                    }
            }

            // clear the accumulated time
            // should I record the duration in total?
            m_AccuTime = 0.0;
        }
    }
}

void AttachMagic::Draw(int nDrawOffX, int nDrawOffY)
{
    if(RefreshCache()){
        if(m_CacheEntry->GfxID >= 0){
            extern SDLDevice *g_SDLDevice;
            extern PNGTexOffDBN *g_MagicDBN;

            int nOffX = 0;
            int nOffY = 0;
            if(auto pEffectTexture = g_MagicDBN->Retrieve(m_CacheEntry->GfxID + Frame(), &nOffX, &nOffY)){
                SDL_SetTextureBlendMode(pEffectTexture, SDL_BLENDMODE_ADD);
                g_SDLDevice->DrawTexture(pEffectTexture, nDrawOffX + nOffX, nDrawOffY + nOffY);
            }
        }
    }
}

bool AttachMagic::Done() const
{
    if(StageDone()){
        if(RefreshCache()){
            switch(m_CacheEntry->Stage){
                case EGS_INIT:
                    {
                        switch(ID()){
                            case DBCOM_MAGICID(u8"雷电术"):
                            case DBCOM_MAGICID(u8"召唤骷髅"):
                                {
                                    return true;
                                }
                            default:
                                {
                                    break;
                                }
                        }
                        break;
                    }
                case EGS_DONE:
                    {
                        return true;
                    }
                default:
                    {
                        break;
                    }
            }
        }else{
            // when we deref m_CacheEntry
            // we should call RefreshCache() first

            // when really done Update() will make current stage as EGS_NONE
            // then RefreshCache() makes m_CacheEntry as nullptr
            return true;
        }
    }
    return false;
}

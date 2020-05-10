/*
 * =====================================================================================
 *
 *       Filename: attachmagic.cpp
 *        Created: 08/10/2017 12:46:45
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
#include "pngtexoffdb.hpp"

extern Log *g_log;
extern SDLDevice *g_SDLDevice;
extern PNGTexOffDB *g_magicDB;

AttachMagic::AttachMagic(int nMagicID, int nMagicParam, int nMagicStage, double fLastTime)
    : MagicBase(nMagicID, nMagicParam, nMagicStage, fLastTime)
{
    if(RefreshCache()){
        switch(m_cacheEntry->Type){
            case EGT_BOUND:
                {
                    break;
                }
            default:
                {
                    g_log->addLog(LOGTYPE_FATAL, "Invalid GfxEntry::Type to AttachMagic");
                    m_cacheEntry->Print();
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
        m_accuTime += fTime;

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
                        switch(ID()){
                            case DBCOM_MAGICID(u8"雷电术"):
                            case DBCOM_MAGICID(u8"魔法盾"):
                                {
                                    m_stage = EGS_DONE;
                                    break;
                                }
                            default:
                                {
                                    if(fnCheckStageValid(EGS_RUN)){
                                        m_stage = EGS_RUN;
                                    }else if(fnCheckStageValid(EGS_DONE)){
                                        m_stage = EGS_DONE;
                                    }else{
                                        m_stage = EGS_NONE;
                                    }
                                    break;
                                }
                        }
                        break;
                    }
                case EGS_START:
                    {
                        if(fnCheckStageValid(EGS_RUN)){
                            m_stage = EGS_RUN;
                        }else if(fnCheckStageValid(EGS_DONE)){
                            m_stage = EGS_DONE;
                        }else{
                            m_stage = EGS_NONE;
                        }
                        break;
                    }
                case EGS_RUN:
                    {
                        if(fnCheckStageValid(EGS_DONE)){
                            m_stage = EGS_DONE;
                        }else{
                            m_stage = EGS_NONE;
                        }
                        break;
                    }
                case EGS_DONE:
                    {
                        m_stage = EGS_NONE;
                        break;
                    }
                default:
                    {
                        break;
                    }
            }

            // clear the accumulated time
            // should I record the duration in total?
            m_accuTime = 0.0;
        }
    }
}

void AttachMagic::Draw(int nDrawOffX, int nDrawOffY)
{
    if(RefreshCache()){
        if(m_cacheEntry->GfxID >= 0){
            int nOffX = 0;
            int nOffY = 0;
            if(auto pEffectTexture = g_magicDB->Retrieve(m_cacheEntry->GfxID + Frame(), &nOffX, &nOffY)){
                SDL_SetTextureBlendMode(pEffectTexture, SDL_BLENDMODE_BLEND);
                g_SDLDevice->DrawTexture(pEffectTexture, nDrawOffX + nOffX, nDrawOffY + nOffY);
            }
        }
    }
}

bool AttachMagic::Done() const
{
    if(StageDone()){
        if(RefreshCache()){
            switch(m_cacheEntry->Stage){
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
            // when we deref m_cacheEntry
            // we should call RefreshCache() first

            // when really done Update() will make current stage as EGS_NONE
            // then RefreshCache() makes m_cacheEntry as nullptr
            return true;
        }
    }
    return false;
}

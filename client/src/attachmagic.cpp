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

#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "dbcomrecord.hpp"
#include "attachmagic.hpp"
#include "pngtexoffdb.hpp"

extern SDLDevice *g_SDLDevice;
extern PNGTexOffDB *g_magicDB;

void AttachMagic::Update(double fTime)
{
    if(!Done()){
        m_accuTime += fTime;

        if(StageDone()){
            auto fnCheckStageValid = [this](int nNewStage) -> bool
            {
                if(auto &rstMR = DBCOM_MAGICRECORD(ID())){
                    for(int nGfxEntryIndex = 0;; ++nGfxEntryIndex){
                        if(auto &rstGfxEntry = rstMR.getGfxEntry(nGfxEntryIndex)){
                            if(rstGfxEntry.stage == nNewStage){
                                return true;
                            }
                        }
                        else{
                            break;
                        }
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
                                    }
                                    else if(fnCheckStageValid(EGS_DONE)){
                                        m_stage = EGS_DONE;
                                    }
                                    else{
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
                        }
                        else if(fnCheckStageValid(EGS_DONE)){
                            m_stage = EGS_DONE;
                        }
                        else{
                            m_stage = EGS_NONE;
                        }
                        break;
                    }
                case EGS_RUN:
                    {
                        if(fnCheckStageValid(EGS_DONE)){
                            m_stage = EGS_DONE;
                        }
                        else{
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

void AttachMagic::Draw(int drawOffX, int drawOffY)
{
    if(refreshCache()){
        if(m_cacheEntry->gfxID >= 0){
            int offX = 0;
            int offY = 0;
            const uint32_t texID = [this]()
            {
                switch(m_cacheEntry->dirType){
                    case 1 : return m_cacheEntry->gfxID + Frame();
                    case 8 : return m_cacheEntry->gfxID + Frame() + (m_direction - DIR_BEGIN) * m_cacheEntry->gfxIDCount;
                    default: throw  fflerror("invalid dirType: %d", m_cacheEntry->dirType);
                }
            }();

            if(auto texPtr = g_magicDB->Retrieve(texID, &offX, &offY)){
                SDL_SetTextureBlendMode(texPtr, SDL_BLENDMODE_BLEND);
                g_SDLDevice->drawTexture(texPtr, drawOffX + offX, drawOffY + offY);
            }
        }
    }
}

bool AttachMagic::Done() const
{
    if(StageDone()){
        if(refreshCache()){
            switch(m_cacheEntry->stage){
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
            // we should call refreshCache() first

            // when really done Update() will make current stage as EGS_NONE
            // then refreshCache() makes m_cacheEntry as nullptr
            return true;
        }
    }
    return false;
}

/*
 * =====================================================================================
 *
 *       Filename: effectnode.cpp
 *        Created: 08/08/2017 12:43:18
 *  Last Modified: 08/08/2017 16:17:27
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

#include "sdldevice.hpp"
#include "effectnode.hpp"
#include "pngtexoffdbn.hpp"

EffectNode::EffectNode(int nMagicID,
        int nMagicParam,
        int nGfxEntryID,
        double fTimeOut)
    : m_MagicID(nMagicID)
    , m_MagicParam(nMagicParam)
    , m_GfxEntryID(nGfxEntryID)
    , m_TimeOut(fTimeOut)
      , m_AccuTime(0.0)
{}

EffectNode::EffectNode(int nMagicID, int nMagicParam, int nGfxEntryID)
    : EffectNode(nMagicID, nMagicParam, nGfxEntryID, -1.0)
{}

void EffectNode::Update(double fTime)
{
    m_AccuTime += fTime;
}

int EffectNode::Frame() const
{
    if(auto &rstGfxEntry = DBCOM_MAGICRECORD(MagicID()).GetGfxEntry(GfxEntryID())){
        auto nRealFrame = (int)(std::lround((m_AccuTime / 1000.0) * SYS_DEFFPS * (rstGfxEntry.Speed / 100.0)));
        if(rstGfxEntry.Loop == 1){
            return nRealFrame % rstGfxEntry.FrameCount;
        }else{
            return nRealFrame;
        }
    }
    return -1;
}

void EffectNode::Draw(int nDrawOffX, int nDrawOffY)
{
    if(!Done()){
        if(auto &rstGfxEntry = DBCOM_MAGICRECORD(MagicID()).GetGfxEntry(GfxEntryID())){
            if(rstGfxEntry.GfxID >= 0){
                extern SDLDevice *g_SDLDevice;
                extern PNGTexOffDBN *g_MagicDBN;

                int nOffX = 0;
                int nOffY = 0;
                if(auto pEffectTexture = g_MagicDBN->Retrieve(rstGfxEntry.GfxID + Frame(), &nOffX, &nOffY)){
                    SDL_SetTextureBlendMode(pEffectTexture, SDL_BLENDMODE_ADD);
                    g_SDLDevice->DrawTexture(pEffectTexture, nDrawOffX + nOffX, nDrawOffY + nOffY);
                }
            }
        }
    }
}

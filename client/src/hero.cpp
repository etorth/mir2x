/*
 * =====================================================================================
 *
 *       Filename: hero.cpp
 *        Created: 9/3/2015 3:49:00 AM
 *  Last Modified: 04/03/2017 17:05:35
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

#include "hero.hpp"
#include "sysconst.hpp"
#include "sdldevice.hpp"
#include "pngtexoffdbn.hpp"

Hero::Hero(uint32_t nUID, uint32_t nDBID, bool bMale, uint32_t nDressID, ProcessRun *pRun, int nX, int nY, int nAction, int nDirection, int nSpeed)
    : Creature(nUID, pRun, nX, nY, nAction, nDirection, nSpeed)
    , m_DBID(nDBID)
    , m_Male(bMale)
    , m_DressID(nDressID)
{}

void Hero::Draw(int nViewX, int nViewY)
{
    auto nGfxID = GfxID();
    if(nGfxID >= 0){
        // human gfx indexing
        // 04 - 00 :     frame : max =  32
        // 07 - 05 : direction : max =  08 : +
        // 13 - 08 :     state : max =  64 : +----> GfxID
        // 21 - 14 :     dress : max = 256 
        //      22 :       sex :
        //      23 :    shadow :
        uint32_t nKey0 = ((uint32_t)(1) << 23) + (((uint32_t)(m_Male ? 1 : 0)) << 22) + ((uint32_t)(m_DressID & 0XFF) << 14) + (((uint32_t)(nGfxID & 0X01FF)) << 5) + m_Frame; // body
        uint32_t nKey1 = ((uint32_t)(0) << 23) + (((uint32_t)(m_Male ? 1 : 0)) << 22) + ((uint32_t)(m_DressID & 0XFF) << 14) + (((uint32_t)(nGfxID & 0X01FF)) << 5) + m_Frame; // body

        int nDX0 = 0;
        int nDY0 = 0;
        int nDX1 = 0;
        int nDY1 = 0;

        extern PNGTexOffDBN *g_HeroGfxDBN;
        auto pFrame0 = g_HeroGfxDBN->Retrieve(nKey0, &nDX0, &nDY0);
        auto pFrame1 = g_HeroGfxDBN->Retrieve(nKey1, &nDX1, &nDY1);

        int nShiftX = 0;
        int nShiftY = 0;
        EstimatePixelShift(&nShiftX, &nShiftY);

        extern SDLDevice *g_SDLDevice;
        if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, 128); }
        g_SDLDevice->DrawTexture(pFrame1, m_X * SYS_MAPGRIDXP + nDX1 - nViewX + nShiftX, m_Y * SYS_MAPGRIDYP + nDY1 - nViewY + nShiftY);
        g_SDLDevice->DrawTexture(pFrame0, m_X * SYS_MAPGRIDXP + nDX0 - nViewX + nShiftX, m_Y * SYS_MAPGRIDYP + nDY0 - nViewY + nShiftY);
    }
}

void Hero::Update()
{
}

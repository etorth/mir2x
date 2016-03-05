/*
 * =====================================================================================
 *
 *       Filename: resource.cpp
 *        Created: 03/05/2016 03:58:44
 *  Last Modified: 03/05/2016 04:03:55
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

#include "game.hpp"

bool Game::LoadTokenBoard(TokenBoard *pBoard, tinyxml2::XMLDocument *pDoc)
{
    if(pBoard == nullptr || pDoc == nullptr){ return false; }

    // what to load for a tokenboard?
    // 1. W, H
    // 2. H1, H2
    // 3. FPS, frameCount

    auto fnBoxSize = [this](bool bUTF8CharBox, uint64_t nKey, int &nW, int &nH){
        auto p = bUTF8CharBox ? m_FontexDB->Retrieve(nKey) : m_EmoticonDB->Retrieve((uint32_t)nKey);
        if(p){
            SDL_QueryTexture(p, nullptr, nullptr, &nW, &nH);
            return true;
        }
        return false;
    };

    auto fnEmoticonInfo = [this](uint32_t nKey, int &nFPS, int &nFrameCount, int &nHRatio){
        return m_EmoticonDB->RetrieveAttribute(nKey, nFPS, nFrameCount, nHRatio);
    };

    return pBoard->Load(pDoc, fnBoxSize, fnEmoticonInfo);
}

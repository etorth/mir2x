#include <cstring>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "texturemanager.hpp"
#include "devicemanager.hpp"
#include <string>
#include "utf8char.hpp"
#include <unordered_map>
#include "configurationmanager.hpp"
#include "fonttexturemanager.hpp"
#include "../../dirent-1.20.1/include/dirent.h"
#include <array>

FontexDB::FontexDB(const std::array<256, std::string> &szFontFileNameV)
    : m_FontFileNameV(szFontFileNameV)
{
}

TTF_Font *FontexDB::LoadFont(uint8_t nFileIndex, uint8_t nSize)
{
    return TTF_OpenFont(m_FontFileNameV[nFileIndex].c_str(), nSize);
}


bool FontexDB::LinearCacheRetrieve(
        uint32_t nFontFaceKey,
        uint32_t nUTF8Code, SDL_Texture * &pTexture)
{
    uint8_t nLCKey = (uint8_t)(nFontFaceKey + nUTF8Code + 7);
    for(m_LCache[nLCKey].Reset(); !m_LCache[nLCKey].Done(); m_LCache.Forward()){
        if(std::get<1>(m_LCache[nLCKey].Current()) == nFontFaceKey
                && std::get<2>(m_LCache[nLCKey].Current()) == nUTF8Code){
            
            pTexture = std::get<0>(m_LCache[nLCKey].Current());
            m_LCache[nLCKey].SwapHead();

            m_TimeStampQ.push({m_CurrentTime, nFontFaceKey, nUTF8Code});


            return true;
        }
    }
    return false;
}

SDL_Texture *FontexDB::Retrieve(uint8_t nFileIndex, uint8_t nSize, uint8_t nStyle, uint32_t nUTF8Code)
{
    uint32_t nFontFaceKey = ((uint32_t)nFileIndex << 16) + ((uint32_t)nSize << 8) + nStyle;
    uint8_t  nLCacheKey   = (uint8_t)(nFontFaceKey + nUTF8Code + 7);

    SDL_Texture *pTexture;

    if(LinearCacheRetrieve(nLCacheKey, nFontFaceKey, nUTF8Code, pTexture)){
        return pTexture;
    }

    auto pFontFaceInst = m_FontFaceSet.find(nFontFaceKey);
    if(pFontFaceInst != m_FontFaceSet.end()){
        pTextureInst = pFontFaceInst.second.find(nUTF8Code);
        if(pTextureInst != pFontFaceInst.second.end()){
            m_LCache[nLCKey].PushHead(nFontFaceKey, nUTF8Code, pTextureInst.second);
            return pTextureInst.second;
        }
    }

    // otherwise load it
    pTexture = LoadTexture(nFileIndex, nSize, nStyle, nUTF8Code);
    m_FontFaceSet[nFontFaceKey][nUTF8Code] = pTexture;
    m_LCache[nLCKey].PushHead(nFontFaceKey, nUTF8Code, pTexture);

    return pTexture;
}




SDL_Texture *FontexDB::Retrieve(uint8_t nFileIndex, uint8_t nSize, uint8_t nStyle, uint32_t nUTF8Code)
{
    uint32_t nFontFaceKey = ((uint32_t)nFileIndex << 16) + ((uint32_t)nSize << 8) + nStyle;
    uint8_t  nLCacheKey   = (uint8_t)(nFontFaceKey + nUTF8Code + 7);

    SDL_Texture *pTexture;

    if(LinearCacheRetrieve(nLCacheKey, nFontFaceKey, nUTF8Code, pTexture)){
        return pTexture;
    }

    auto pFontFaceInst = m_FontFaceSet.find(nFontFaceKey);
    if(pFontFaceInst != m_FontFaceSet.end()){
        pTextureInst = pFontFaceInst.second.find(nUTF8Code);
        if(pTextureInst != pFontFaceInst.second.end()){
            m_LCache[nLCKey].PushHead(nFontFaceKey, nUTF8Code, pTextureInst.second);
            return pTextureInst.second;
        }
    }

    // otherwise load it
    pTexture = LoadTexture(nFileIndex, nSize, nStyle, nUTF8Code);
    m_FontFaceSet[nFontFaceKey][nUTF8Code] = pTexture;
    m_LCache[nLCKey].PushHead(nFontFaceKey, nUTF8Code, pTexture);

    return pTexture;
}

bool FontexDB::Init()
{
    if(!TTF_WasInit() && TTF_Init() == -1) {
        SDL_Log("Could not initialize TTF: %s", TTF_GetError());
        SDL_Quit();
        exit(0);
    }

    // Load all fonts filename, not font file itself
    auto pFont = GetConfigurationManager()->GetXMLElement("Root/Font");
    if(pFont != nullptr){
        auto pFontDesc = pFont->FirstChildElement("FontFile");
        int  fontIndex = 0;
        while(pFontDesc){
            if(pFontDesc->Attribute("Index", std::to_string(fontIndex).c_str())){
                auto pFileName = pFontDesc->FirstChildElement("FileName");
                if(pFileName){
                    m_FontFileName[fontIndex] = pFileName->GetText();
                }else{
                    break;
                }
            }else{
                break;
            }
            fontIndex++;
            pFontDesc = pFontDesc->NextSiblingElement("FontFile");
        }
    }
    return true;
}

void FontexDB::Release()
{
    // TODO
}

SDL_Texture *FontexDB::RetrieveTexture(const UTF8CHARTEXTUREINDICATOR &stCharIdtor)
{
    std::string szKey((const char *)(&stCharIdtor), sizeof(stCharIdtor));
    if(m_UTF8CharTextureCache.find(szKey) == m_UTF8CharTextureCache.end()){
        auto pTexture = LoadTexture(stCharIdtor);
        if(pTexture == nullptr){
            return nullptr;
        }else{
            m_UTF8CharTextureCache[szKey].first  = 1;
            m_UTF8CharTextureCache[szKey].second = pTexture;
            return pTexture;
        }
    }else{
        m_UTF8CharTextureCache[szKey].first++;
        return m_UTF8CharTextureCache[szKey].second;
    }
}

SDL_Texture *FontexDB::CreateTexture(const FONTINFO &stFontInfo, const SDL_Color &stColor, const char *szInfo)
{
    TTF_Font *pFont  = nullptr;
    uint32_t  nIndex = stFontInfo.Index;
    uint32_t  nSize  = stFontInfo.Size;

    std::string szFontKey = std::to_string(nIndex) + "#" + std::to_string(nSize);
    if(m_TTFFontCache.find(szFontKey) != m_TTFFontCache.end()){
        pFont = m_TTFFontCache[szFontKey];
    }else{
        pFont = TTF_OpenFont(m_FontFileName[nIndex].c_str(), nSize);
        if(pFont == nullptr){
            return nullptr;
        }else{
            TTF_SetFontKerning(pFont, false);
            m_TTFFontCache[szFontKey] = pFont;
        }
    }

    SDL_Surface *pSurface = nullptr;
    if(stFontInfo.Style & FONTSTYLE_SOLID){
        pSurface = TTF_RenderUTF8_Solid(pFont, szInfo, stColor);
    }else if(stFontInfo.Style & FONTSTYLE_SHADED){
        pSurface = TTF_RenderUTF8_Shaded(pFont, szInfo, stColor, {0xff, 0xff, 0xff});
    }else{
        pSurface = TTF_RenderUTF8_Blended(pFont, szInfo, stColor);
    }

    if(pSurface){
        SDL_Texture *pTexture = 
            SDL_CreateTextureFromSurface(GetDeviceManager()->GetRenderer(), pSurface);
        SDL_FreeSurface(pSurface);
        m_UTF8LineTextureCache.push_back(pTexture);
        return pTexture;
    }
    return nullptr;
}


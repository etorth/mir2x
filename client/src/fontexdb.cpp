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

FontexDB *GetFontTextureManager()
{
    static FontexDB fontTextureManager;
    return &fontTextureManager;
}

FontexDB::FontexDB(const std::array<256, std::string> &szFontFileNameV)
    : m_FontFileNameV(szFontFileNameV)
{
}

TTF_Font *FontexDB::LoadFont(uint8_t nFileIndex, uint8_t nSize)
{
    return TTF_OpenFont(m_FontFileNameV[nFileIndex].c_str(), nSize);
}

TTF_Font *FontexDB::RetrieveFont(uint8_t nFileIndex, uint8_t nSize)
{
    // uint8_t nFontHashCode = (nFileIndex & 0X0F) + (nSize << 4);
    uint16_t nFontCode = ((uint16_t)nFileIndex << 8) + nSize;
    auto p = m_FontCache.find(nFontCode);
    if(p != m_FontCache.end()){
        // p.second maybe valid or nullptr
        // for nullptr means we loaded it before but failed
        // this suppress to load every time
        return p.second;
    }

    auto pFont = LoadFont(nFileIndex, nSize);
    m_FontCache[nFontCode] = pFont;
    return pFont;
}

TTF_Font *FontexDB::RetrieveDefaultFont(uint8_t nSize)
{
    for(int nIndex = 0; nIndex < 256; ++nIndex){
        auto pFont = RetrieveFont(nIndex, nSize);
        if(pFont){ return pFont; }
    }
    return nullptr;
}

SDL_Texture *FontexDB::Retrieve(uint8_t nFileIndex,
        uint8_t nSize, uint8_t nStyle, uint32_t nColor, uint32_t nUTF8Code)
{
    auto pFont = RetrieveFont(nFileIndex, nSize);
    if(pFont == nullptr){
        pFont = RetrieveDefaultFont(nIndex, nSize);
    }

    if(pFont == nullptr){ return nullptr; }

    return RetrieveTexture(pFont, nStyle, nColor, nUTF8Code);
}

SDL_Texture *FontexDB::RetrieveTexture(TTF_Font *pFont,
        uint8_t nStyle, uint32_t nColor, uint32_t nUTF8Code)
{

}
    auto pUTF8Set = m_UTF8FontCache.find((uintptr_t)pFont);
    if(pUTF8Set != m_UTF8FontCache.end()){
        auto pTexture = pUTF8Set.second[nStyle].find(nColor);
        if(pTexture != pUTF8Set.second[nStyle].end()){
            return pTexture.second;
        }
    }

    auto pTex = LoadTexture
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

SDL_Texture *FontexDB::LoadTexture(const UTF8CHARTEXTUREINDICATOR &stCharIdtor)
{
    // we need to create a new texture in this function
    // so don't check texture cache in this function
    // but we may need to check font cache
    //
    TTF_Font *pFont  = nullptr;
    uint32_t  nIndex = stCharIdtor.FontInfo.Index;
    uint32_t  nSize  = stCharIdtor.FontInfo.Size;

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
    if(stCharIdtor.FontInfo.Style & FONTSTYLE_SOLID){
        pSurface = TTF_RenderUTF8_Solid(pFont, stCharIdtor.Data, stCharIdtor.Color);
    }else if(stCharIdtor.FontInfo.Style & FONTSTYLE_SHADED){
        pSurface = TTF_RenderUTF8_Shaded(pFont, stCharIdtor.Data, stCharIdtor.Color, {0xff, 0xff, 0xff});
    }else{
        pSurface = TTF_RenderUTF8_Blended(pFont, stCharIdtor.Data, stCharIdtor.Color);
    }

    if(pSurface){
        SDL_Texture *pTexture = 
            SDL_CreateTextureFromSurface(GetDeviceManager()->GetRenderer(), pSurface);
        SDL_FreeSurface(pSurface);
        return pTexture;
    }
    return nullptr;
}

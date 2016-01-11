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

FontTextureManager *GetFontTextureManager()
{
    static FontTextureManager fontTextureManager;
    return &fontTextureManager;
}

bool FontTextureManager::Init()
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

void FontTextureManager::Release()
{
    // TODO
}

SDL_Texture *FontTextureManager::RetrieveTexture(const UTF8CHARTEXTUREINDICATOR &stCharIdtor)
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

SDL_Texture *FontTextureManager::CreateTexture(const FONTINFO &stFontInfo, const SDL_Color &stColor, const char *szInfo)
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

SDL_Texture *FontTextureManager::LoadTexture(const UTF8CHARTEXTUREINDICATOR &stCharIdtor)
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

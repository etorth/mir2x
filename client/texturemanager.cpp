#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <SDL_image.h>
#include <SDL.h>
#include "texturemanager.hpp"
#include "configurationmanager.hpp"
#include "devicemanager.hpp"
#include <string>
#include <unordered_map>
#include <array>
#include "misc.hpp"

TextureManager::TextureManager()
	: m_TexturePath("")
{}

TextureManager::~TextureManager()
{
	for(auto &p : m_TextureCache){
		SDL_DestroyTexture(p.second);
	}
}

SDL_Texture *TextureManager::RetrieveTexture(
        uint32_t nPrecode, uint32_t nFileIndex, uint32_t nImageIndex)
{
    nPrecode    = (nPrecode    & 0X000003F);
    nFileIndex  = (nFileIndex  & 0X000003FF);
    nImageIndex = (nImageIndex & 0X0000FFFF);

    uint32_t nU32Key = (nPrecode << 26) + (nFileIndex << 16) + (nImageIndex);
    if(m_TextureCache.find(nU32Key) != m_TextureCache.end()){
        return m_TextureCache[nU32Key];
    }else{
        char szPNGFileName[16];
        std::sprintf(szPNGFileName, "%02d%04d%05d.PNG", nPrecode, nFileIndex, nImageIndex);
        std::string szPNGFullName = m_TexturePath + "/" + szPNGFileName;
        // maybe nullptr, but that's OK
        // means we skip some error code to load time by time
        SDL_Texture *pTexture = LoadSDLTextureFromFile(
                szPNGFullName.c_str(), GetDeviceManager()->GetRenderer());
        m_TextureCache[nU32Key] = pTexture;

        return pTexture;
    }
}

SDL_Texture *TextureManager::RetrieveTexture(uint32_t nU32Key)
{
    return RetrieveTexture((nU32Key & 0XFC000000) >> 26,
            (nU32Key & 0X03FF0000) >> 16, (nU32Key & 0X0000FFFF));
}

TextureManager *GetTextureManager()
{
    static TextureManager textureManager;
    return &textureManager;
}

bool TextureManager::Init()
{
    m_TexturePath = GetConfigurationManager()->GetString("Root/Texture/Path");
    return true;
}

void TextureManager::ClearCache()
{
    for(auto &p: m_TextureCache){
        SDL_DestroyTexture(p.second);
    }
}

void TextureManager::Release()
{
    ClearCache();
}

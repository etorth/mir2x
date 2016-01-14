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

MapTextureManager::MapTextureManager()
	: m_MapTexturePath("")
    : m_MapTextureMaxCount(2000)
{}

MapTextureManager::~MapTextureManager()
{
	for(auto &p : m_TextureCache){
		SDL_DestroyTexture(p.second);
	}
}

SDL_Texture *MapTextureManager::RetrieveTexture(uint32_t nU32Key)
{
    uint32_t nTimeMS = GetTimeManager()->Now();
    if(m_MapTextureCache.find(nU32Key) != m_MapTextureCache.end()){
        return m_MapTextureCache[nU32Key];
    }else{

    }
    uint32_t nU32Key = (nPrecode << 26) + (nFileIndex << 16) + (nImageIndex);
    if(m_TextureCache.find(nU32Key) != m_TextureCache.end()){
        return m_TextureCache[nU32Key];
    }else{
        char szPNGFileName[16];
        std::sprintf(szPNGFileName, "%02d%04d%05d.PNG", nPrecode, nFileIndex, nImageIndex);
        std::string szPNGFullName = m_MapTexturePath + "/" + szPNGFileName;
        // maybe nullptr, but that's OK
        // means we skip some error code to load time by time
        SDL_Texture *pTexture = LoadSDLTextureFromFile(
                szPNGFullName.c_str(), GetDeviceManager()->GetRenderer());
        m_TextureCache[nU32Key] = pTexture;

        return pTexture;
    }
}

SDL_Texture *MapTextureManager::RetrieveTexture(uint32_t nU32Key)
{
    return RetrieveTexture((nU32Key & 0XFC000000) >> 26,
            (nU32Key & 0X03FF0000) >> 16, (nU32Key & 0X0000FFFF));
}

MapTextureManager *GetTextureManager()
{
    static MapTextureManager textureManager;
    return &textureManager;
}

bool MapTextureManager::Init()
{
    m_MapTexturePath = GetConfigurationManager()->GetString("Root/Texture/Path");
    return true;
}

void MapTextureManager::ClearCache()
{
    for(auto &p: m_TextureCache){
        SDL_DestroyTexture(p.second);
    }
}

void MapTextureManager::Release()
{
    ClearCache();
}

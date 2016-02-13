#pragma once
#include "imageaccess.hpp"
#include "maptexturemanager.hpp"

MapTextureManager *GetMapTextureManager()
{
    static MapTextureManager textureManager;
    return &textureManager;
}

bool MapTextureManager::Init()
{
    m_MapTexturePath = GetConfigurationManager()->GetString("Root/Map/Texture");
}

SDL_Texture *MapTextureManager::Load(const uint32_t &nU32Key)
{
    char szPNGFileName[16];
    std::sprintf(szPNGFileName, "%05d%05d.PNG", nU32Key >> 16, nU32Key & 0X0000FFFF);
    std::string szPNGFullName = m_MapTexturePath + "/" + szPNGFileName;
    return LoadSDLTextureFromFile(szPNGFullName.c_str(), GetDeviceManager()->GetRenderer());
}

void MapTextureManager::Release(SDL_Texture *pTexture)
{
    if(pTexture){
        SDL_DestroyTexture(pTexture);
    }
}

#include <cstring>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "texturemanager.hpp"
#include "devicemanager.hpp"
#include <string>
#include "utf8char.hpp"
#include <unordered_map>
#include "configurationmanager.hpp"
#include "../../dirent-1.20.1/include/dirent.h"
#include "emoticonmanager.hpp"

EmoticonManager *GetEmoticonManager()
{
    static EmoticonManager textureManager;
    return &textureManager;
}

bool EmoticonManager::Init()
{
    // Load all emoticons, just path and filename
    auto szEmoticonPath = GetConfigurationManager()->GetString("Root/Emoticon/Path");
    if(szEmoticonPath == nullptr){
        szEmoticonPath = "Res/Emoticon";
    }

    // Open directory stream
    auto stDir = opendir(szEmoticonPath);
    if(stDir != nullptr){
        // Print all files and directories within the directory
        struct dirent *szEnt = nullptr;
        readdir(stDir);
        readdir(stDir);

        int nSet = 0;
        while((szEnt = readdir(stDir)) != nullptr){
            switch(szEnt->d_type){
                case DT_DIR:
                    m_EmoticonFilePathCache.push_back({});
                    LoadEmoticonSet((std::string(szEmoticonPath) + "/" + szEnt->d_name).c_str(), nSet++);
                    break;

                default:
                    // ignore all others
                    break;
            }
        }
        closedir(stDir);
    }else{
        // Could not open directory
        // No emoticons successfully loaded
        // printf ("Cannot open directory %s\n", szEmoticonPath);
    }

    // make two other cache with the same size of emoticon file path cache
    m_EmoticonInfoCache = std::vector<
        std::vector<int>>(m_EmoticonFilePathCache.size(), {});
    m_EmoticonXMLDesc   = std::vector<
        std::vector<tinyxml2::XMLDocument *>>(m_EmoticonFilePathCache.size(), {});
    m_EmoticonFrameInfoCache = std::vector<
        std::vector<std::vector<EMOTICONFRAMEINFO>>>(m_EmoticonFilePathCache.size(), {});
    m_EmoticonFrameTextureCache = std::vector<
        std::vector<std::vector<SDL_Texture *>>>(m_EmoticonFilePathCache.size(), {});
    for(int nSet = 0; nSet < m_EmoticonFilePathCache.size(); ++nSet){
        m_EmoticonXMLDesc[nSet] = std::vector<
            tinyxml2::XMLDocument *>(m_EmoticonFilePathCache[nSet].size(), nullptr);
        m_EmoticonInfoCache[nSet] = std::vector<
            int>(m_EmoticonFilePathCache[nSet].size(), -1);
        m_EmoticonFrameInfoCache[nSet] = std::vector<
            std::vector<EMOTICONFRAMEINFO>>(m_EmoticonFilePathCache[nSet].size(), {});
        m_EmoticonFrameTextureCache[nSet] = std::vector<
            std::vector<SDL_Texture *>>(m_EmoticonFilePathCache[nSet].size(), {});
    }
    return true;
}

void EmoticonManager::LoadEmoticonSet(const char *szEmoticonPath, int nSet)
{
    // TODO: Load name of emoticonset
    auto stDir = opendir(szEmoticonPath);
    if(stDir){
        // Print all files and directories within the directory
        struct dirent *szEnt = nullptr;

        readdir(stDir);
        readdir(stDir);

        while((szEnt = readdir(stDir)) != nullptr){
            switch(szEnt->d_type){
                case DT_DIR:
                    {
                        m_EmoticonFilePathCache[nSet].push_back(
                                std::string(szEmoticonPath) + "/" + szEnt->d_name);
                        break;
                    }
                default:
                    // ignore all others
                    break;
            }
        }
        closedir(stDir);
    }else{
        // Could not open directory
        // No emoticons successfully loaded
        // printf ("Cannot open directory %s\n", szEmoticonPath);
    }
}

void EmoticonManager::Release()
{
}

SDL_Texture *EmoticonManager::RetrieveTexture(int nSet, int nIndex, int nFrameIndex)
{
    if(m_EmoticonFrameTextureCache[nSet][nIndex].size() == 0){
        LoadEmoticonFrame(nSet, nIndex);
    }
    return m_EmoticonFrameTextureCache[nSet][nIndex][nFrameIndex];
}

void EmoticonManager::LoadEmoticonFrameTexture(int nSet, int nIndex, const char *szFileName)
{
    auto pTextureIterator = m_EmoticonFrameTextureCachePool.find(szFileName);
    if(pTextureIterator != m_EmoticonFrameTextureCachePool.end()){
        m_EmoticonFrameTextureCache[nSet][nIndex].push_back(pTextureIterator->second);
    }else{
        SDL_Surface *pSurface = IMG_Load(szFileName);
        SDL_Texture *pTexture = nullptr;
        if(pSurface){
            pTexture = SDL_CreateTextureFromSurface(GetDeviceManager()->GetRenderer(), pSurface);
            SDL_FreeSurface(pSurface);
        }
        m_EmoticonFrameTextureCachePool[szFileName] = pTexture;
        m_EmoticonFrameTextureCache[nSet][nIndex].push_back(pTexture);
    }
}

const EMOTICONFRAMEINFO &EmoticonManager::RetrieveEmoticonFrameInfo(int nSet, int nIndex, int nFrameIndex)
{
    if(m_EmoticonFrameInfoCache[nSet][nIndex].size() == 0){
        LoadEmoticonFrame(nSet, nIndex);
    }
    return m_EmoticonFrameInfoCache[nSet][nIndex][nFrameIndex];
}

// when in Init(), only load emoticon path
// LoadEmoticonInfo() only load general info of a emoticon
// here load emoticon fully: EMOTICONFRAMEINFO, Texture
void EmoticonManager::LoadEmoticonFrame(int nSet, int nIndex)
{
    if(m_EmoticonInfoCache[nSet][nIndex] < 0){
        LoadEmoticonInfo(nSet, nIndex);
    }

    std::vector<EMOTICONFRAMEINFO> vEmoticonFrameInfo;

    auto pRoot   = m_EmoticonXMLDesc[nSet][nIndex]->RootElement();
    auto pConent = pRoot->FirstChildElement("Content");
    auto pFrame  = pConent->FirstChildElement("Frame");
    while(pFrame != nullptr){
        auto pRes = pFrame->FirstChildElement("Res");
        auto pX   = pFrame->FirstChildElement("X");
        auto pY   = pFrame->FirstChildElement("Y");
        auto pW   = pFrame->FirstChildElement("W");
        auto pH   = pFrame->FirstChildElement("H");
        auto pDX  = pFrame->FirstChildElement("DX");
        auto pDY  = pFrame->FirstChildElement("DY");

        pFrame = pFrame->NextSiblingElement("Frame");

        EMOTICONFRAMEINFO stEFInfo;
        stEFInfo.X  = std::atoi(pX ->GetText());
        stEFInfo.Y  = std::atoi(pY ->GetText());
        stEFInfo.W  = std::atoi(pW ->GetText());
        stEFInfo.H  = std::atoi(pH ->GetText());
        stEFInfo.DX = std::atoi(pDX->GetText());
        stEFInfo.DY = std::atoi(pDY->GetText());
        vEmoticonFrameInfo.push_back(stEFInfo);

        LoadEmoticonFrameTexture(nSet, nIndex, std::string(
                    m_EmoticonFilePathCache[nSet][nIndex] + "/" + pRes->GetText()).c_str());
    }
    m_EmoticonFrameInfoCache[nSet][nIndex] = std::move(vEmoticonFrameInfo);
}

void EmoticonManager::LoadEmoticonInfo(int nSet, int nIndex)
{
    m_EmoticonXMLDesc[nSet][nIndex] = new tinyxml2::XMLDocument();
    m_EmoticonXMLDesc[nSet][nIndex]->LoadFile((m_EmoticonFilePathCache[nSet][nIndex] + "/desc.xml").c_str());

    auto pRoot   = m_EmoticonXMLDesc[nSet][nIndex]->RootElement();
    auto pW      = pRoot->FirstChildElement("W");
    auto pH      = pRoot->FirstChildElement("H");
    auto pBL     = pRoot->FirstChildElement("BL");
    auto pFPS    = pRoot->FirstChildElement("FPS");

    EMOTICONINFO stEInfo;
    stEInfo.W          = std::atoi(pW->GetText());
    stEInfo.H          = std::atoi(pH->GetText());
    stEInfo.H2         = std::atoi(pBL->GetText());
    stEInfo.H1         = stEInfo.H - stEInfo.H2;
    stEInfo.FPS        = std::atoi(pFPS->GetText());
    stEInfo.Method     = 0; // TODO: set blit method for emoticon

    m_EmoticonInfoCachePool.push_back(stEInfo);
    m_EmoticonInfoCache[nSet][nIndex] = m_EmoticonInfoCachePool.size() - 1;
}

const EMOTICONINFO &EmoticonManager::RetrieveEmoticonInfo(int nSet, int nIndex)
{
    if(m_EmoticonInfoCache[nSet][nIndex] < 0){
        LoadEmoticonInfo(nSet, nIndex);
    }
    return m_EmoticonInfoCachePool[m_EmoticonInfoCache[nSet][nIndex]];
}

int EmoticonManager::RetrieveEmoticonFrameCount(int nSet, int nIndex)
{
    // can not be zero
    if(m_EmoticonFrameInfoCache[nSet][nIndex].size() == 0){
        LoadEmoticonFrame(nSet, nIndex);
    }
    return m_EmoticonFrameInfoCache[nSet][nIndex].size();
}

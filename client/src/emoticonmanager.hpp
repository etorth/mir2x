#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "emoticon.hpp"
#include <unordered_map>
#include <vector>
#include <tinyxml2.h>

class EmoticonManager
{
    private:
        EmoticonManager() = default;

    public:
        bool Init();
        void Release();

    public:
        SDL_Texture             *RetrieveTexture(int, int, int);
        const EMOTICONINFO      &RetrieveEmoticonInfo(int, int);
        const EMOTICONFRAMEINFO &RetrieveEmoticonFrameInfo(int, int, int);
        int                      RetrieveEmoticonFrameCount(int, int);

    private:
        void LoadEmoticonSet(const char *, int);
        void LoadEmoticonInfo(int, int);
        void LoadEmoticonFrame(int, int);
        void LoadEmoticonFrameTexture(int, int, const char *);

    private:
        std::vector<std::vector<int>>                            m_EmoticonInfoCache;
        std::vector<EMOTICONINFO>                                m_EmoticonInfoCachePool;
        std::vector<std::vector<std::string>>                    m_EmoticonFilePathCache;
        std::vector<std::vector<std::vector<EMOTICONFRAMEINFO>>> m_EmoticonFrameInfoCache;
        std::vector<std::vector<std::vector<SDL_Texture *>>>     m_EmoticonFrameTextureCache;
        std::unordered_map<std::string, SDL_Texture *>           m_EmoticonFrameTextureCachePool;
        std::vector<std::vector<tinyxml2::XMLDocument *>>        m_EmoticonXMLDesc;

    public:
        friend EmoticonManager *GetEmoticonManager();
};

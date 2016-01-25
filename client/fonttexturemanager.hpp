#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "emoticon.hpp"
#include <unordered_map>
#include "utf8char.hpp"
#include <vector>
#include <array>
#include <map>


class FontTextureManager
{
    private:
        FontTextureManager() = default;

    public:
        bool Init();
        void Release();

    public:
        SDL_Texture *RetrieveTexture(const UTF8CHARTEXTUREINDICATOR &);
    public:
        // CreateTexture() will keep the texture for you
        // so you can use it without worrying about free it
        // but CreateTexture() won't check redundancy of the texture
        // it always create a new texture and return
        // you need to check it by yourself
        //
        // this is because we have more information to check the redundancy: ID, state etc.
        // than string comparing the manager have only
        //
        // support utf8
        // seems never used, just put it here
        SDL_Texture *CreateTexture(const FONTINFO &, const SDL_Color &, const char *);
    private:
        SDL_Texture *LoadTexture(const UTF8CHARTEXTUREINDICATOR &);

    private:
        std::unordered_map<std::string, std::pair<int, SDL_Texture*>> m_UTF8CharTextureCache;
        std::vector<SDL_Texture *>                                    m_UTF8LineTextureCache;

    private:
        std::unordered_map<int,          std::string> m_FontFileName;
        std::unordered_map<std::string,    TTF_Font*> m_TTFFontCache;

    public:
        friend FontTextureManager *GetFontTextureManager();

};

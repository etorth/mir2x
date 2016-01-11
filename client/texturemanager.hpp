#pragma once
#include <SDL.h>
#include <unordered_map>

class TextureManager final
{
    private:
        TextureManager();
        ~TextureManager();

    private:
        TextureManager(const TextureManager &)              = delete;
        TextureManager &operator = (const TextureManager &) = delete;

    private:
        std::string m_TexturePath;

    public:
        bool Init();
        void Release();

    public:
        SDL_Texture *RetrieveTexture(uint32_t, uint32_t, uint32_t);
        SDL_Texture *RetrieveTexture(uint32_t);

    public:
        void ClearCache();

    private:
        std::unordered_map<uint32_t, SDL_Texture*> m_TextureCache;

    public:
        friend TextureManager *GetTextureManager();
};

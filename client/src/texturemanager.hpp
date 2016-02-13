#pragma once
#include <SDL2/SDL.h>
#include <unordered_map>

class MapTextureManager final
{
    private:
        MapTextureManager();
        ~MapTextureManager();

    private:
        MapTextureManager(const MapTextureManager &)              = delete;
        MapTextureManager &operator = (const MapTextureManager &) = delete;

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

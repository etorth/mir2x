#pragma once

#include <SDL.h>
#include "imageaccess.hpp"
#include "sourcemanager.hpp"

class MapTextureManager: public SourceManager<uint32_t, SDL_Texture>
{
    private:
        MapTextureManager()  = default;
        ~MapTextureManager() = default;

    private:
        MapTextureManager(const MapTextureManager &)              = delete;
        MapTextureManager &operator = (const MapTextureManager &) = delete;

    private:
        std::string m_MapTexturePath;

    public:
        bool Init();

    public:
        virtual SDL_Texture *Load(const uint32_t &);
        virtual void Release(SDL_Texture *);

    public:
        friend MapTextureManager *GetMapTextureManager();
};

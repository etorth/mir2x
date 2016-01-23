#pragma once

#include <SDL.h>
#include "imageaccess.hpp"
#include "sourcemanager.hpp"

class GUITextureManager: public SourceManager<uint32_t, SDL_Texture>
{
    private:
        GUITextureManager()  = default;
        ~GUITextureManager() = default;

    private:
        GUITextureManager(const GUITextureManager &)              = delete;
        GUITextureManager &operator = (const GUITextureManager &) = delete;

    private:
        std::string m_MapTexturePath;

    public:
        bool Init();

    public:
        virtual SDL_Texture *Load(const uint32_t &);
        virtual void Release(SDL_Texture *);

    public:
        friend GUITextureManager *GetGUITextureManager();
};

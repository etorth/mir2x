#pragma once
#include <cstring>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "zsdb.hpp"
#include "inndb.hpp"

// SDL_mixer supports only 1 Mix_Music playing
// when a Mix_music gets freed, SDL_mixer dynamically checks if it's playing

struct BGMusicElement
{
    Mix_Music *music = nullptr;
    std::vector<uint8_t> musicFileData; // seems SDL_mixer access data during playing
};

class BGMusicDB: public innDB<uint32_t, BGMusicElement>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    public:
        BGMusicDB(size_t resMax)
            : innDB<uint32_t, BGMusicElement>(resMax)
        {}

    public:
        virtual ~BGMusicDB() = default;

    public:
        bool load(const char *bgmDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(bgmDBName);
            return true;
        }

    public:
        Mix_Music *retrieve(uint32_t key)
        {
            if(auto p = innLoad(key)){
                return p->music;
            }
            return nullptr;
        }

    public:
        std::optional<std::tuple<BGMusicElement, size_t>> loadResource(uint32_t) override;
        void freeResource(BGMusicElement &element) override;
};

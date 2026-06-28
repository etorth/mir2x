#pragma once
#include <cstring>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "zsdb.hpp"
#include "inndb.hpp"

struct BGMusicElement
{
    MIX_Audio *music = nullptr;
    std::vector<uint8_t> musicFileData; // SDL_mixer streams from this buffer
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
        void load(const char *bgmDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(bgmDBName);
        }

    public:
        MIX_Audio *retrieve(uint32_t key)
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

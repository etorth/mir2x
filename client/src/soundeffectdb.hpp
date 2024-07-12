#pragma once
#include <cstring>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "soundeffecthandle.hpp"

struct SoundEffectElement
{
    // SDL_mixer accesses data during playing
    // internal database needs to keep both chunk pointer and file data in shared_ptr
    std::shared_ptr<SoundEffectHandle> handle = nullptr;
};

class SoundEffectDB: public innDB<uint32_t, SoundEffectElement>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    public:
        SoundEffectDB(size_t resMax)
            : innDB<uint32_t, SoundEffectElement>(resMax)
        {}

    public:
        virtual ~SoundEffectDB() = default;

    public:
        bool load(const char *soundEffectDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(soundEffectDBName);
            return true;
        }

    public:
        std::shared_ptr<SoundEffectHandle> retrieve(uint32_t key)
        {
            // several channel can play the same chunk
            // this requires Mix_PlayChannel() RO-access to chunk data

            if(auto p = innLoad(key)){
                return p->handle;
            }
            return nullptr;
        }

    public:
        std::optional<std::tuple<SoundEffectElement, size_t>> loadResource(uint32_t) override;
        void freeResource(SoundEffectElement &element) override;
};

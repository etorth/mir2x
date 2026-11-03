#pragma once
#include <vector>
#include <memory>
#include <utility>
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "hexstr.hpp"
#include "sdldevice.hpp"

// proguse UI images
// each key is one APNG, a static image is a 1-frame APNG
//
//  static image : frame 0 only
//  button group : frame 0/1/2 for off/on/down
//  animation    : frame 0..N-1, with per-frame delay in ms

struct ProgUseElement
{
    std::vector<std::pair<SDL_Texture *, int>> frameList {};
};

class ProgUseDB final: public innDB<uint32_t, ProgUseElement>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    public:
        ProgUseDB(size_t resMax)
            : innDB<uint32_t, ProgUseElement>(resMax)
        {}

    public:
        ~ProgUseDB() override
        {
            clear();
        }

    public:
        void load(const char *texDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(texDBName);
        }

    public:
        // returns nullptr if no such key or frame out of range
        SDL_Texture *retrieve(uint32_t key, int frame = 0)
        {
            if(auto p = innLoad(key); p && frame >= 0 && frame < std::ssize(p->frameList)){
                return p->frameList[frame].first;
            }
            return nullptr;
        }

        int frameCount(uint32_t key)
        {
            if(auto p = innLoad(key)){
                return std::ssize(p->frameList);
            }
            return 0;
        }

        // delay in ms, returns 0 if no such key or frame out of range
        int frameDelay(uint32_t key, int frame)
        {
            if(auto p = innLoad(key); p && frame >= 0 && frame < std::ssize(p->frameList)){
                return p->frameList[frame].second;
            }
            return 0;
        }

    public:
        std::optional<std::tuple<ProgUseElement, size_t>> loadResource(uint32_t) override;

    public:
        void freeResource(ProgUseElement &) override;
};

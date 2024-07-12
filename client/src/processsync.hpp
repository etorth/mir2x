#pragma once
#include <SDL2/SDL.h>
#include "process.hpp"
#include "labelboard.hpp"

class ProcessSync: public Process
{
    private:
        int        m_ratio;
        LabelBoard m_processBarInfo;

    public:
        ProcessSync();

    public:
        ~ProcessSync() = default;

    public:
        int id() const override
        {
            return PROCESSID_SYRC;
        }

    public:
        void draw() const override;
        void update(double) override;
        void processEvent(const SDL_Event &) override;
};

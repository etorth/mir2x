#pragma once
#include <SDL2/SDL.h>
#include "process.hpp"

class ProcessLogo: public Process
{
    private:
        const double m_timeR1 = 0.3;
        const double m_timeR2 = 0.3;

    private:
        const double m_fullTime = 5000.0;

    private:
        double m_totalTime = 0.0;

    public:
        ProcessLogo(): Process() {}

    public:
        virtual ~ProcessLogo() = default;

    public:
        int id() const override
        {
            return PROCESSID_LOGO;
        }

    public:
        void draw() const override;
        void update(double) override;
        void processEvent(const SDL_Event &) override;

    private:
        double colorRatio() const;
};

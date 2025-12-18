#pragma once
#include <SDL2/SDL.h>
#include "process.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "gfxcropboard.hpp"

class ProcessSync: public Process
{
    private:
        int m_ratio = 0;

    private:
        Widget m_canvas;

    private:
        ImageBoard m_barFull;
        GfxCropBoard m_bar;

    private:
        ImageBoard m_bgImg;

    private:
        TextBoard m_barText;

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

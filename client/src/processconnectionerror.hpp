#pragma once
#include "process.hpp"
#include "inputstringboard.hpp"

class ProcessConnectionError: public Process
{
    private:
        InputStringBoard m_board;

    public:
        ProcessConnectionError();

    public:
        int id() const override
        {
            return PROCESSID_CONNECTIONERROR;
        }

        void draw() const override;
        void update(double) override;
        void processEvent(const SDL_Event &) override;
};

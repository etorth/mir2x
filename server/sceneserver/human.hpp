#pragma once
#include "actor.hpp"

class Human: public Actor
{
    public:
        Human(int, int, int);
        ~Human();

    public:
        void Update();

    private:
        int m_Level;

    public:
        void SetLevel(int);
};

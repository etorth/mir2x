#pragma once
#include "hero.hpp"

class MyHero: public Hero
{
    public:
        MyHero(int, int, int);
        ~MyHero();

    public:
        void Update();
};

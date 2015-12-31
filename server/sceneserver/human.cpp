#include "human.hpp"
#include <cstdio>

Human::Human(int nSID, int nUID, int nGenTime)
    : Actor(nSID, nUID, nGenTime)
    , m_Level(0)
{}

Human::~Human()
{}

void Human::Update()
{
    // printf("human updated here...\n");
}

void Human::SetLevel(int nLevel)
{
    m_Level = nLevel;
}

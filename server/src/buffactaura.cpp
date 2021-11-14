#include "buffactaura.hpp"

BaseBuffActAura::BaseBuffActAura(uint32_t argBuffID, uint32_t argBuffActID)
    : BaseBuffAct(argBuffID, argBuffActID)
{}

BaseBuffActAura::~BaseBuffActAura()
{}

BaseBuffActAura *BaseBuffActAura::createAura(uint32_t argBuffID, uint32_t argBuffActID)
{
    return new BaseBuffActAura(argBuffID, argBuffActID);
}

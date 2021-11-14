#include "buffactspellmodifier.hpp"

BaseBuffActSpellModifier::BaseBuffActSpellModifier(uint32_t argBuffID, uint32_t argBuffActID)
    : BaseBuffAct(argBuffID, argBuffActID)
{}

BaseBuffActSpellModifier *BaseBuffActSpellModifier::createSpellModifier(uint32_t argBuffID, uint32_t argBuffActID)
{
    return new BaseBuffActSpellModifier(argBuffID, argBuffActID);
}

#include "buff.hpp"
#include "buffact.hpp"
#include "buffactspellmodifier.hpp"

BaseBuffActSpellModifier::BaseBuffActSpellModifier(BaseBuff *argBuff, size_t argBuffActOff)
    : BaseBuffAct(argBuff, argBuffActOff)
{
    fflassert(getBAR().isSpellModifier());
}

BaseBuffActSpellModifier *BaseBuffActSpellModifier::createSpellModifier(BaseBuff *argBuff, size_t argBuffActOff)
{
    return new BaseBuffActSpellModifier(argBuff, argBuffActOff);
}

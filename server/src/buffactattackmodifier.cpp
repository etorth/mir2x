#include "strf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "buff.hpp"
#include "buffact.hpp"
#include "buffactattackmodifier.hpp"

BaseBuffActAttackModifier::BaseBuffActAttackModifier(BaseBuff *argBuff, size_t argBuffActOff)
    : BaseBuffAct(argBuff, argBuffActOff)
    , m_roller([this]() -> float
      {
          const auto prob = getBAREF().attackModifier.prob;
          fflassert(prob >= 0);
          fflassert(prob <= 100);
          return to_f(prob) / 100.0;
      }())
{
    fflassert(getBAR().isAttackModifier());
}

uint32_t BaseBuffActAttackModifier::rollBuff()
{
    if(str_haschar(getBAR().attackModifier.buff)){
        const auto id = DBCOM_BUFFID(getBAR().attackModifier.buff);
        const auto &br = DBCOM_BUFFRECORD(id);

        fflassert(id);
        fflassert(br);

        if(m_roller.roll()){
            return id;
        }
    }
    return 0;
}

uint32_t BaseBuffActAttackModifier::rollModifier()
{
    if(str_haschar(getBAR().attackModifier.modifier)){
        const auto id = DBCOM_ATTACKMODIFIERID(getBAR().attackModifier.modifier);
        const auto &br = DBCOM_ATTACKMODIFIERRECORD(id);

        fflassert(id);
        fflassert(br);

        if(m_roller.roll()){
            return id;
        }
    }
    return 0;
}

BaseBuffActAttackModifier *BaseBuffActAttackModifier::createAttackModifier(BaseBuff *argBuff, size_t argBuffActOff)
{
    return new BaseBuffActAttackModifier(argBuff, argBuffActOff);
}

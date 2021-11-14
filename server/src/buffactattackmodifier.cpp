#include "strf.hpp"
#include "fflerror.hpp"
#include "buffactattackmodifier.hpp"

BaseBuffActAttackModifier::BaseBuffActAttackModifier(uint32_t argBuffID, uint32_t argBuffActID)
    : BaseBuffAct(argBuffID, argBuffActID)
    , m_roller([this]() -> float
      {
          fflassert(getBAREF().attackModifier.prob >= 0);
          fflassert(getBAREF().attackModifier.prob <= 100);
          return getBAREF().attackModifier.prob / 100.0f;
      }())
{}

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

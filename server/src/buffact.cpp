#include "fflerror.hpp"
#include "buffact.hpp"
#include "battleobject.hpp"

BaseBuffAct::BaseBuffAct(BaseBuff *argBuff, size_t argBuffActOff)
    : m_buff([argBuff]()
      {
          fflassert(argBuff);
          return argBuff;
      }())
    , m_actOff([argBuff, argBuffActOff]()
      {
          fflassert(argBuffActOff < argBuff->getBR().actList.size());
          return argBuffActOff;
      }())

    , m_id([argBuff, argBuffActOff]()
      {
          const auto id = DBCOM_BUFFACTID(argBuff->getBR().actList.begin()[argBuffActOff].name);
          const auto &bar = DBCOM_BUFFACTRECORD(id);

          fflassert(id);
          fflassert(bar);
          return id;
      }())
{}

bool BaseBuffAct::done() const
{
    switch(getBAREF().duration){
        case BADUR_UNLIMITED:
            {
                return false;
            }
        case BADUR_INSTANT:
            {
                return true;
            }
        default:
            {
                return std::lround(m_buff->accuTime()) >= getBAREF().duration;
            }
    }
}

BaseBuffAct *BaseBuffAct::createBuffAct(BaseBuff *argBuff, size_t argBuffActOff)
{
    fflassert(argBuff);
    fflassert(argBuffActOff < argBuff->getBR().actList.size());

    const auto id = DBCOM_BUFFACTID(argBuff->getBR().actList.begin()[argBuffActOff].name);
    const auto &bar = DBCOM_BUFFACTRECORD(id);

    fflassert(id);
    fflassert(bar);

    if(bar.isAura()){
        return BaseBuffActAura::createAura(argBuff, argBuffActOff);
    }

    if(bar.isController()){
        return BaseBuffActController::createController(argBuff, argBuffActOff);
    }

    if(bar.isAttributeModifier()){
        return BaseBuffActAttributeModifier::createAttributeModifier(argBuff, argBuffActOff);
    }

    if(bar.isAttackModifier()){
        return BaseBuffActAttackModifier::createAttackModifier(argBuff, argBuffActOff);
    }

    if(bar.isTrigger()){
        return BaseBuffActTrigger::createTrigger(argBuff, argBuffActOff);
    }

    throw fflvalue(argBuff, argBuffActOff, uidf::getUIDString(argBuff->getBO()->UID()), argBuff->getBR().name, argBuff->getBR().actList.begin()[argBuffActOff].name);
}

const BuffRecord & BaseBuffAct::getBR() const
{
    return getBuff()->getBR();
}

const BuffActRecord & BaseBuffAct::getBAR() const
{
    return DBCOM_BUFFACTRECORD(id());
}

const BuffRecord::BuffActRecordRef & BaseBuffAct::getBAREF() const
{
    return getBuff()->getBR().actList.begin()[m_actOff];
}

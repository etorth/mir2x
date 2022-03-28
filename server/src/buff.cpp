#include <memory>
#include "uidf.hpp"
#include "fflerror.hpp"
#include "buff.hpp"
#include "buffact.hpp"
#include "battleobject.hpp"

BaseBuff::BaseBuff(BattleObject *argBO, uint64_t argFromUID, uint64_t argFromBuffSeq, uint32_t argBuffID, uint32_t argSeqID)
    : m_bo([argBO]()
      {
          fflassert(argBO);
          switch(uidf::getUIDType(argBO->UID())){
              case UID_MON:
              case UID_PLY: return argBO;
              default: throw fflvalue(uidf::getUIDString(argBO->UID()));
          }
      }())
    , m_fromUID([argFromUID]()
      {
          switch(uidf::getUIDType(argFromUID)){
              case UID_MON:
              case UID_PLY: return argFromUID;
              default: throw fflvalue(uidf::getUIDString(argFromUID));
          }
      }())
    , m_fromBuffSeq(argFromBuffSeq)
    , m_id([argBuffID]()
      {
          fflassert(argBuffID);
          fflassert(DBCOM_BUFFRECORD(argBuffID));
          return argBuffID;
      }())
    , m_seqID([argSeqID]()
      {
          fflassert(argSeqID);
          return argSeqID;
      }())
{
    for(size_t buffActOff = 0; const auto &baref: getBR().actList){
        fflassert(baref, getBR().name, buffActOff);
        m_actList.insert(std::unique_ptr<BaseBuffAct>(BaseBuffAct::createBuffAct(this, buffActOff++)));
    }
}

BaseBuff::~BaseBuff()
{}

void BaseBuff::runOnUpdate()
{
    // update buff
    // currenlty only trigger needs update

    for(auto p = m_actList.begin(); p != m_actList.end();){
        if(p->get()->getBAR().isTrigger()){
            fflassert(validBuffActTrigger(p->get()->getBAREF().trigger.on));
            dynamic_cast<BaseBuffActTrigger *>(p->get())->checkTimedTrigger();
        }

        if(p->get()->done()){
            p = m_actList.erase(p);
        }
        else{
            p++;
        }
    }
}

void BaseBuff::runOnTrigger(int btgr)
{
    fflassert(validBuffActTrigger(btgr));
    for(auto &actPtr: m_actList){
        if(actPtr->getBAR().isTrigger()){
            fflassert(validBuffActTrigger(actPtr->getBAREF().trigger.on));
            auto ptgr = dynamic_cast<BaseBuffActTrigger *>(actPtr.get());
            fflassert(ptgr);

            for(int m = 1; m < BATGR_END; m <<= 1){
                if(actPtr->getBAREF().trigger.on & m){
                    ptgr->runOnTrigger(m);
                }
            }
        }
    }
}

void BaseBuff::runOnBOMove()
{
    // BO has moved
    // check if need to disable because out of radius

    if(getBO()->UID() == fromUID()){
        return;
    }

    // capture this->getBO() should be fine, but don't capture *this*
    // buff may get released before lambada triggered

    if(const auto bap = fromAuraBAREF()){
        getBO()->addDelay(0, [boPtr = getBO(), fromUID = fromUID(), buffSeq = buffSeq(), radius = bap->aura.radius]() // may call removeBuff() and can break outside for-loop
        {
            boPtr->getCOLocation(fromUID, [boPtr, buffSeq, radius](const COLocation &coLoc)
            {
                if((boPtr->mapID() != coLoc.mapID) || (mathf::LDistance2<int>(boPtr->X(), boPtr->Y(), coLoc.x, coLoc.y) > radius * radius)){
                    boPtr->removeBuff(buffSeq, true);
                }
            });
        });
    }
}

void BaseBuff::runOnDone()
{
}

std::vector<BaseBuffActAura *> BaseBuff::getAuraList()
{
    std::vector<BaseBuffActAura *> result;
    for(auto &actPtr: m_actList){
        if(actPtr->getBAR().isAura()){
            result.push_back(dynamic_cast<BaseBuffActAura *>(actPtr.get()));
        }
    }
    return result;
}

void BaseBuff::sendAura(uint64_t uid)
{
    for(auto paura: getAuraList()){
        paura->transmit(uid);
    }
}

void BaseBuff::dispatchAura()
{
    for(auto paura: getAuraList()){
        paura->dispatch();
    }
}

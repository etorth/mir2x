#include <memory>
#include "uidf.hpp"
#include "fflerror.hpp"
#include "buff.hpp"
#include "buffact.hpp"
#include "dbcomrecord.hpp"
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
    m_runList.reserve(getBR().actList.size());
    for(size_t buffActOff = 0; const auto &baref: getBR().actList){
        fflassert(baref);
        m_runList.push_back(BuffActRunner
        {
            .tpsCount = 0,
            .ptr = std::unique_ptr<BaseBuffAct>(BaseBuffAct::createBuffAct(this, buffActOff++)),
        });
    }
}

BaseBuff::~BaseBuff()
{}

void BaseBuff::runOnUpdate()
{
    for(auto &[tpsCount, ptr]: m_runList){
        if(ptr->getBAR().isTrigger()){
            fflassert(validBuffActTrigger(ptr->getBAREF().trigger.on));
            if(ptr->getBAREF().trigger.on & BATGR_TIME){
                auto ptgr = dynamic_cast<BaseBuffActTrigger *>(ptr.get());
                fflassert(ptgr);

                const auto neededCount = std::lround(m_accuTime * ptr->getBAREF().trigger.tps / 1000.0);
                while(tpsCount++ < neededCount){
                    ptgr->runOnTrigger(BATGR_TIME);
                }
            }
        }
    }
}

void BaseBuff::runOnTrigger(int btgr)
{
    fflassert(validBuffActTrigger(btgr));
    for(auto &[tpsCount, ptr]: m_runList){
        if(ptr->getBAR().isTrigger()){
            fflassert(validBuffActTrigger(ptr->getBAREF().trigger.on));
            auto ptgr = dynamic_cast<BaseBuffActTrigger *>(ptr.get());
            fflassert(ptgr);

            for(int m = 1; m < BATGR_END; m <<= 1){
                if(ptr->getBAREF().trigger.on & m){
                    ptgr->runOnTrigger(m);
                }
            }
        }
    }
}

void BaseBuff::runOnMove()
{
    if(fromAuraBAREF()){
        if(getBO()->UID() == fromUID()){
            return;
        }

        getBO()->addDelay(0, [this]() // may call removeBuff() and can break outside for-loop
        {
            getBO()->getCOLocation(fromUID(), [this](const COLocation &coLoc)
            {
                const auto bap = fromAuraBAREF();
                fflassert(bap);

                if((getBO()->mapID() != coLoc.mapID) || (mathf::LDistance2<int>(getBO()->X(), getBO()->Y(), coLoc.x, coLoc.y) > bap->aura.radius * bap->aura.radius)){
                    getBO()->removeBuff(buffSeq(), true);
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
    for(auto &run: m_runList){
        if(run.ptr->getBAR().isAura()){
            auto paura = dynamic_cast<BaseBuffActAura *>(run.ptr.get());
            fflassert(paura);
            result.push_back(paura);
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

#include <memory>
#include "uidf.hpp"
#include "fflerror.hpp"
#include "buff.hpp"
#include "buffact.hpp"
#include "dbcomrecord.hpp"
#include "battleobject.hpp"

BaseBuff::BaseBuff(BattleObject *argBO, uint64_t argFromUID, uint32_t argBuffID)
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
    , m_id([argBuffID]()
      {
          fflassert(argBuffID);
          fflassert(DBCOM_BUFFRECORD(argBuffID));
          return argBuffID;
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
        else if(ptr->getBAR().isAura()){
            auto paura = dynamic_cast<BaseBuffActAura *>(ptr.get());
            fflassert(paura);
            paura->transmit();
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

void BaseBuff::runOnDone()
{
}

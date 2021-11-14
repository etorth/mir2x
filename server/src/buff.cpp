#include "uidf.hpp"
#include "fflerror.hpp"
#include "buff.hpp"
#include "buffact.hpp"
#include "dbcomrecord.hpp"
#include "battleobject.hpp"

BaseBuff::BaseBuff(uint32_t argBuffID, BattleObject *argBO)
    : m_id(argBuffID)
    , m_bo([argBO]()
      {
          fflassert(argBO);
          switch(uidf::getUIDType(argBO->UID())){
              case UID_MON:
              case UID_PLY: return argBO;
              default: throw fflvalue(uidf::getUIDString(argBO->UID()));
          }
      }())
{
    m_runList.reserve(getBR().actList.size());
    for(const auto &baref: getBR().actList){
        fflassert(baref);
        m_runList.push_back(BuffActRunner
        {
            .tpsCount = 0,
            .ptr = std::unique_ptr<BaseBuffAct>(BaseBuffAct::createBuffAct(argBuffID, DBCOM_BUFFACTID(baref.name))),
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
                    ptgr->runOnTrigger(m_bo, BATGR_TIME);
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
                    ptgr->runOnTrigger(m_bo, m);
                }
            }
        }
    }
}

void BaseBuff::runOnDone()
{
}

#include "uidf.hpp"
#include "fflerror.hpp"
#include "buff.hpp"
#include "dbcomrecord.hpp"
#include "bufftrigger.hpp"
#include "buffmodifier.hpp"
#include "battleobject.hpp"

BaseBuff::BaseBuff(uint32_t id, BattleObject *bo)
    : m_id(id)
    , m_br([id]() -> const auto &
      {
          const auto &br = DBCOM_BUFFRECORD(id);
          fflassert(br);
          return br;
      }())
    , m_bo([bo]()
      {
          fflassert(bo);
          switch(uidf::getUIDType(bo->UID())){
              case UID_MON:
              case UID_PLY: return bo;
              default: throw bad_value(uidf::getUIDString(bo->UID()));
          }
      }())
{
    m_modList.reserve(m_br.modList.size());
    for(const auto &[type, arg]: m_br.modList){
        fflassert(validBuffModifier(type));
        m_modList.push_back(std::make_unique<BaseBuffModifier>(m_bo, type, arg));
    }

    m_tgrList.reserve(m_br.tgrList.size());
    for(const auto &[name, arg]: m_br.tgrList){
        const auto tgrId = DBCOM_BUFFTRIGGERID(name);
        fflassert(tgrId > 0);
        fflassert(tgrId < DBCOM_BUFFTRIGGERENDID());
        m_tgrList.push_back(std::make_tuple(0, BaseBuffTrigger::createTrigger(tgrId, arg)));
    }
}

BaseBuff::~BaseBuff()
{
}

void BaseBuff::runOnUpdate()
{
    for(auto &[tpsCount, trigger]: m_tgrList){
        fflassert(trigger);
        fflassert(tpsCount >= 0);
        const auto &tr = DBCOM_BUFFTRIGGERRECORD(trigger->id());

        fflassert(tr);
        if(tr.on | BTGR_TIME){
            for(const auto needed = std::lround(m_accuTime * tr.tps / 1000.0); tpsCount < needed; ++tpsCount){
                trigger->runOnTrigger(m_bo, BTGR_TIME);
            }
        }
    }
}

void BaseBuff::runOnTrigger(int btgr)
{
    fflassert(validBuffTrigger(btgr));
    for(auto &[tpsCount, trigger]: m_tgrList){
        fflassert(trigger);
        const auto &tr = DBCOM_BUFFTRIGGERRECORD(trigger->id());

        fflassert(tr);
        fflassert(validBuffTrigger(tr.on));
        for(int m = 1; m < BTGR_END; m <<= 1){
            if(tr.on | m){
                trigger->runOnTrigger(m_bo, m);
            }
        }
    }
}

#include <memory>
#include <algorithm>
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
        m_activeActList.push_back(std::unique_ptr<BaseBuffAct>(BaseBuffAct::createBuffAct(this, buffActOff++)));
    }
}

corof::awaitable<> BaseBuff::runOnTrigger(int btgr)
{
    fflassert(validBuffActTrigger(btgr));

    // trigger may suspend, this buff can be removed meanwhile
    // don't touch *this* after co_await, always look up the act again by its key

    auto bo = getBO();
    std::vector<std::pair<uint64_t, size_t>> keyList;

    for(auto &actPtr: m_activeActList){
        if(actPtr->getBAR().isTrigger()){
            keyList.push_back(actPtr->actKey());
        }
    }

    for(const auto &key: keyList){
        for(int m = 1; m < BATGR_END; m <<= 1){
            if(!(btgr & m)){
                continue;
            }

            if(auto ptgr = dynamic_cast<BaseBuffActTrigger *>(bo->m_buffList.hasBuffAct(key))){
                if(ptgr->getBAREF().trigger.on & m){
                    co_await ptgr->runOnTrigger(m);
                }
            }
            else{
                break;
            }
        }
    }
}

corof::awaitable<> BaseBuff::runOnBOMove()
{
    // BO has moved
    // check if need to disable because out of radius

    if(getBO()->UID() == fromUID()){
        co_return;
    }

    // capture this->getBO() should be fine, but don't capture *this*
    // buff may get released before lambada triggered

    if(const auto bap = fromAuraBAREF()){
        auto bo = getBO();
        auto seq = buffSeq();

        // aura source is gone, or out of radius
        const auto coLocOpt = co_await bo->getCOLocation(fromUID());
        if(!coLocOpt.has_value() || (bo->mapUID() != coLocOpt.value().mapUID) || (mathf::LDistance2<int>(bo->X(), bo->Y(), coLocOpt.value().x, coLocOpt.value().y) > bap->aura.radius * bap->aura.radius)){
            bo->removeBuff(seq, true);
        }
    }
}

corof::awaitable<> BaseBuff::runOnDone()
{
    co_return;
}

std::vector<BaseBuffActAura *> BaseBuff::getAuraList()
{
    std::vector<BaseBuffActAura *> result;
    for(auto &actPtr: m_activeActList){
        if(actPtr->getBAR().isAura()){
            result.push_back(dynamic_cast<BaseBuffActAura *>(actPtr.get()));
        }
    }
    return result;
}

std::vector<std::pair<uint64_t, size_t>> BaseBuff::getAuraKeyList()
{
    std::vector<std::pair<uint64_t, size_t>> result;
    for(auto paura: getAuraList()){
        result.push_back(paura->actKey());
    }
    return result;
}

corof::awaitable<> BaseBuff::sendAura(uint64_t uid)
{
    auto bo = getBO();
    for(const auto &key: getAuraKeyList()){
        if(auto paura = dynamic_cast<BaseBuffActAura *>(bo->m_buffList.hasBuffAct(key))){
            co_await paura->transmit(uid);
        }
    }
}

corof::awaitable<> BaseBuff::dispatchAura()
{
    auto bo = getBO();
    for(const auto &key: getAuraKeyList()){
        if(auto paura = dynamic_cast<BaseBuffActAura *>(bo->m_buffList.hasBuffAct(key))){
            co_await paura->dispatch();
        }
    }
}

BaseBuffAct *BaseBuff::hasBuffAct(size_t argActOff) const
{
    for(auto &p: m_activeActList){
        if(p && p->actOff() == argActOff){
            return p.get();
        }
    }
    return nullptr;
}

bool BaseBuff::removeBuffAct(size_t argActOff)
{
    // removed act is not destroyed immediately, caller may still be inside the act
    // BattleObject calls BuffList::clearDeadBuff() later by defer()

    if(auto p = std::find_if(m_activeActList.begin(), m_activeActList.end(), [argActOff](const auto &actPtr){ return actPtr->actOff() == argActOff; }); p != m_activeActList.end()){
        m_deadActList.push_back(std::move(*p));
        m_activeActList.erase(p);
        return true;
    }
    return false;
}

#include "uidf.hpp"
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "friendtype.hpp"
#include "buffactaura.hpp"
#include "battleobject.hpp"

BaseBuffActAura::BaseBuffActAura(BaseBuff *argBuff, size_t argBuffActOff)
    : BaseBuffAct(argBuff, argBuffActOff)
    , m_auraBuffID([this]()
      {
          fflassert(getBAR().isAura());
          const auto id = DBCOM_BUFFID(getBAR().aura.buff);
          const auto &br = DBCOM_BUFFRECORD(id);

          fflassert(id);
          fflassert(br);
          return id;
      }())
{}

corof::awaitable<> BaseBuffActAura::dispatch()
{
    // transmit() suspends, this aura can be removed meanwhile
    // don't touch *this* after co_await, always look up the aura again by its key

    const auto bo = getBuff()->getBO();
    const auto key = actKey();

    for(const auto uid: bo->getInViewUIDList()){
        switch(uidf::getUIDType(uid)){
            case UID_PLY:
            case UID_MON:
                {
                    if(auto paura = dynamic_cast<BaseBuffActAura *>(bo->m_buffList.hasBuffAct(key))){
                        co_await paura->transmit(uid);
                    }
                    else{
                        co_return;
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }
    }
}

corof::awaitable<> BaseBuffActAura::transmit(uint64_t targetUID)
{
    // don't touch *this* after co_await, it can be removed meanwhile

    const auto bo = getBuff()->getBO();
    const auto key = actKey();

    const auto buffSeq = getBuff()->buffSeq();
    const auto auraBuffID = getAuraBuffID();

    const auto favor = getBR().favor;
    const auto radius = getBAREF().aura.radius;

    fflassert(targetUID);
    fflassert(targetUID != bo->UID());

    switch(uidf::getUIDType(targetUID)){
        case UID_PLY:
        case UID_MON:
            {
                const auto coLocOpt = co_await bo->getCOLocation(targetUID);
                if(!coLocOpt.has_value()){
                    co_return;
                }

                if(!bo->m_buffList.hasBuffAct(key)){
                    co_return;
                }

                if(bo->mapUID() != coLocOpt.value().mapUID){
                    co_return;
                }

                if(mathf::LDistance2<int>(bo->X(), bo->Y(), coLocOpt.value().x, coLocOpt.value().y) > radius * radius){
                    co_return;
                }

                const auto friendType = co_await bo->checkFriend(targetUID);
                if(!bo->m_buffList.hasBuffAct(key)){
                    co_return;
                }

                switch(friendType){
                    case FT_FRIEND:
                        {
                            if(favor >= 0){
                                bo->sendBuff(targetUID, buffSeq, auraBuffID);
                            }
                            break;
                        }
                    case FT_ENEMY:
                        {
                            if(favor <= 0){
                                bo->sendBuff(targetUID, buffSeq, auraBuffID);
                            }
                            break;
                        }
                    case FT_NEUTRAL:
                        {
                            bo->sendBuff(targetUID, buffSeq, auraBuffID);
                            break;
                        }
                    default:
                        {
                            throw fflvalue(friendType);
                        }
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

BaseBuffActAura *BaseBuffActAura::createAura(BaseBuff *argBuff, size_t argBuffActOff)
{
    return new BaseBuffActAura(argBuff, argBuffActOff);
}

#include "uidf.hpp"
#include "fflerror.hpp"
#include "friendtype.hpp"
#include "buffactaura.hpp"
#include "battleobject.hpp"

BaseBuffActAura::BaseBuffActAura(BattleObject *argBO, uint32_t argBuffID, uint32_t argBuffActID)
    : BaseBuffAct(argBuffID, argBuffActID)
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
    const auto id = DBCOM_BUFFID(getBAR().aura.buff);
    const auto &br = DBCOM_BUFFRECORD(id);

    fflassert(id);
    fflassert(br);
}

BaseBuffActAura::~BaseBuffActAura()
{}

BaseBuffActAura *BaseBuffActAura::createAura(BattleObject *argBO, uint32_t argBuffID, uint32_t argBuffActID)
{
    return new BaseBuffActAura(argBO, argBuffID, argBuffActID);
}

void BaseBuffActAura::transmitHelper(std::vector<uint64_t> uidList)
{
    while(!uidList.empty()){
        const auto currUID = uidList.back();
        uidList.pop_back();

        switch(uidf::getUIDType(currUID)){
            case UID_PLY:
            case UID_MON:
                {
                    if(currUID != m_bo->UID()){
                        m_bo->checkFriend(currUID, [currUID, uidList = std::move(uidList), this](int friendType) mutable
                        {
                            switch(friendType){
                                case FT_FRIEND:
                                    {
                                        if(DBCOM_BUFFRECORD(getBuffID()).favor >= 0){
                                            m_bo->sendBuff(currUID, getBuffID());
                                        }
                                        break;
                                    }
                                case FT_ENEMY:
                                    {
                                        if(DBCOM_BUFFRECORD(getBuffID()).favor <= 0){
                                            m_bo->sendBuff(currUID, getBuffID());
                                        }
                                        break;
                                    }
                                case FT_NEUTRAL:
                                    {
                                        m_bo->sendBuff(currUID, getBuffID());
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                            transmitHelper(std::move(uidList));
                        });
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

void BaseBuffActAura::transmit()
{
    transmitHelper(m_bo->getInViewUIDList());
}

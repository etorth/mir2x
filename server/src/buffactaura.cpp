#include "uidf.hpp"
#include "fflerror.hpp"
#include "friendtype.hpp"
#include "buffactaura.hpp"
#include "battleobject.hpp"

BaseBuffActAura::BaseBuffActAura(BaseBuff *argBuff, size_t argBuffActOff)
    : BaseBuffAct(argBuff, argBuffActOff)
    , m_auraBuffID([argBuffActOff, this]()
      {
          fflassert(getBAR().isAura());
          const auto id = DBCOM_BUFFID(getBAR().aura.buff);
          const auto &br = DBCOM_BUFFRECORD(id);

          fflassert(id);
          fflassert(br);
          return id;
      }())
{}

BaseBuffActAura::~BaseBuffActAura()
{}

BaseBuffActAura *BaseBuffActAura::createAura(BaseBuff *argBuff, size_t argBuffActOff)
{
    return new BaseBuffActAura(argBuff, argBuffActOff);
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
                    if(currUID != getBuff()->getBO()->UID()){
                        getBuff()->getBO()->checkFriend(currUID, [currUID, uidList = std::move(uidList), this](int friendType) mutable
                        {
                            switch(friendType){
                                case FT_FRIEND:
                                    {
                                        if(getBR().favor >= 0){
                                            getBuff()->getBO()->sendBuff(currUID, getAuraBuffID());
                                        }
                                        break;
                                    }
                                case FT_ENEMY:
                                    {
                                        if(getBR().favor <= 0){
                                            getBuff()->getBO()->sendBuff(currUID, getAuraBuffID());
                                        }
                                        break;
                                    }
                                case FT_NEUTRAL:
                                    {
                                        getBuff()->getBO()->sendBuff(currUID, getAuraBuffID());
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
    transmitHelper(getBuff()->getBO()->getInViewUIDList());
}

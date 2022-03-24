#include "uidf.hpp"
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
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

void BaseBuffActAura::dispatch()
{
    for(const auto uid: getBuff()->getBO()->getInViewUIDList()){
        switch(uidf::getUIDType(uid)){
            case UID_PLY:
            case UID_MON:
                {
                    transmit(uid);
                    break;
                }
            default:
                {
                    break;
                }
        }
    }
}

void BaseBuffActAura::transmit(uint64_t targetUID)
{
    fflassert(targetUID);
    fflassert(targetUID != getBuff()->getBO()->UID());
    switch(uidf::getUIDType(targetUID)){
        case UID_PLY:
        case UID_MON:
            {
                getBuff()->getBO()->getCOLocation(targetUID, [targetUID, this](const COLocation &coLoc)
                {
                    const auto &baref = getBAREF();
                    fflassert(baref);

                    if((getBuff()->getBO()->mapID() != coLoc.mapID) || (mathf::LDistance2<int>(getBuff()->getBO()->X(), getBuff()->getBO()->Y(), coLoc.x, coLoc.y) > baref.aura.radius * baref.aura.radius)){
                        return;
                    }

                    getBuff()->getBO()->checkFriend(targetUID, [targetUID, this](int friendType)
                    {
                        switch(friendType){
                            case FT_FRIEND:
                                {
                                    if(getBR().favor >= 0){
                                        getBuff()->getBO()->sendBuff(targetUID, getBuff()->buffSeq(), getAuraBuffID());
                                    }
                                    break;
                                }
                            case FT_ENEMY:
                                {
                                    if(getBR().favor <= 0){
                                        getBuff()->getBO()->sendBuff(targetUID, getBuff()->buffSeq(), getAuraBuffID());
                                    }
                                    break;
                                }
                            case FT_NEUTRAL:
                                {
                                    getBuff()->getBO()->sendBuff(targetUID, getBuff()->buffSeq(), getAuraBuffID());
                                    break;
                                }
                            default:
                                {
                                    break;
                                }
                        }
                    });
                });
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

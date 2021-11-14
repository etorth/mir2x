#include "fflerror.hpp"
#include "buffact.hpp"
#include "battleobject.hpp"

BaseBuffAct::BaseBuffAct(uint32_t argBuffID, uint32_t argBuffActID)
    : m_buffID([argBuffID]()
      {
          fflassert(argBuffID > 0);
          fflassert(argBuffID < DBCOM_BUFFENDID());
          return argBuffID;
      }())

    , m_buffActID([argBuffActID]()
      {
          fflassert(argBuffActID > 0);
          fflassert(argBuffActID < DBCOM_BUFFACTENDID());
          return argBuffActID;
      }())
{
    fflassert(getBR().hasBuffAct(getBAR().name));
}

BaseBuffAct *BaseBuffAct::createBuffAct(BattleObject *bo, uint32_t argBuffID, uint32_t argBuffActID)
{
    fflassert(argBuffID > 0);
    fflassert(argBuffID < DBCOM_BUFFENDID());

    const auto &br = DBCOM_BUFFRECORD(argBuffID);
    fflassert(br);

    fflassert(argBuffActID > 0);
    fflassert(argBuffActID < DBCOM_BUFFACTENDID());

    const auto &bar = DBCOM_BUFFACTRECORD(argBuffActID);
    fflassert(bar);

    if(bar.isAura             ()) return BaseBuffActAura             ::createAura             (bo, argBuffID, argBuffActID);
    if(bar.isController       ()) return BaseBuffActController       ::createController       (    argBuffID, argBuffActID);
    if(bar.isAttributeModifier()) return BaseBuffActAttributeModifier::createAttributeModifier(bo, argBuffID, argBuffActID);

    throw fflvalue(bo, uidf::getUIDString(bo->UID()), argBuffID, argBuffActID, br.name, bar.name, bar.type);
}

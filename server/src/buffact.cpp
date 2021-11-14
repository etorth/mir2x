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

BaseBuffAct *BaseBuffAct::createBuffAct(uint32_t argBuffID, uint32_t argBuffActID)
{
    fflassert(argBuffID > 0);
    fflassert(argBuffID < DBCOM_BUFFENDID());

    fflassert(argBuffActID > 0);
    fflassert(argBuffActID < DBCOM_BUFFACTENDID());

    const auto &bar = DBCOM_BUFFACTRECORD(argBuffID);
    fflassert(bar);

    if(bar.isAura      ()) return BaseBuffActAura      ::createAura      (argBuffID, argBuffActID);
    if(bar.isController()) return BaseBuffActController::createController(argBuffID, argBuffActID);

    throw fflvalue(argBuffID, argBuffActID, bar.name, bar.type);
}

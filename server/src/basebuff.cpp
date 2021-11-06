#include "uidf.hpp"
#include "fflerror.hpp"
#include "basebuff.hpp"
#include "dbcomrecord.hpp"
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
{}

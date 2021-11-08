#include "buffmodifier.hpp"
#include "battleobject.hpp"

BaseBuffModifier::BaseBuffModifier(BattleObject *bo, int type, int arg)
    : m_type([type]()
      {
          fflassert(validBuffModifier(type));
          return type;
      }())
    , m_sdTaggedVal([bo, type, arg]() -> decltype(m_sdTaggedVal)
      {
          switch(type){
              case BMOD_HP:
                  {
                      bo->m_sdHealth.hp += arg;
                      return std::make_pair(nullptr, 0);
                  }
              case BMOD_MP:
                  {
                      bo->m_sdHealth.mp += arg;
                      return std::make_pair(nullptr, 0);
                  }
              case BMOD_HPMAX:
                  {
                      return std::make_pair(&(bo->m_sdHealth.buffMaxHP), bo->m_sdHealth.buffMaxHP.add(arg));
                  }
              case BMOD_MPMAX:
                  {
                      return std::make_pair(&(bo->m_sdHealth.buffMaxMP), bo->m_sdHealth.buffMaxMP.add(arg));
                  }
              default:
                  {
                      throw bad_value(bo, type, arg);
                  }
          }
      }())
{}

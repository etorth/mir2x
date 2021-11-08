#include "buffmodifier.hpp"
#include "battleobject.hpp"

BaseBuffModifier::BaseBuffModifier(BattleObject *bo, int type, int arg)
    : m_bo([bo]()
      {
          fflassert(bo);
          return bo;
      }())

    , m_type([type]()
      {
          fflassert(validBuffModifier(type));
          return type;
      }())

    , m_tag([type, arg, this]() -> int
      {
          switch(type){
              case BMOD_HP:
                  {
                      m_bo->updateHealth(arg);
                      return 0;
                  }
              case BMOD_MP:
                  {
                      m_bo->updateHealth(0, arg);
                      return 0;
                  }
              case BMOD_HPMAX:
                  {
                      const auto tag = m_bo->m_sdHealth.buffMaxHP.add(arg);
                      m_bo->updateHealth();
                      return tag;
                  }
              case BMOD_MPMAX:
                  {
                      const auto tag = m_bo->m_sdHealth.buffMaxMP.add(arg);
                      m_bo->updateHealth();
                      return tag;
                  }
              default:
                  {
                      throw bad_value(this, type, arg);
                  }
          }
      }())
{}

BaseBuffModifier::~BaseBuffModifier()
{
    switch(m_type){
        case BMOD_HPMAX:
            {
                m_bo->m_sdHealth.buffMaxHP.erase(m_tag);
                break;
            }
        case BMOD_MPMAX:
            {
                m_bo->m_sdHealth.buffMaxMP.erase(m_tag);
                break;
            }
        default:
            {
                break;
            }
    }
    m_bo->updateHealth();
}

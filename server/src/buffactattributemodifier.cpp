#include "monoserver.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
#include "battleobject.hpp"
#include "buffactattributemodifier.hpp"

extern MonoServer *g_monoServer;
BaseBuffActAttributeModifier::BaseBuffActAttributeModifier(BattleObject *bo, uint32_t argBuffID, uint32_t argBuffActID)
    : BaseBuffAct(argBuffID, argBuffActID)
    , m_bo([bo]()
      {
          fflassert(bo);
          return bo;
      }())
    , m_onDone([this]() -> std::function<void()>
      {
          const auto percentage = getBAREF().attributeModifier.percentage;
          const auto value      = getBAREF().attributeModifier.value;

          fflassert(std::abs(percentage) >= 0);
          fflassert(std::abs(percentage) <= 100);

          switch(buffActID()){
              case DBCOM_BUFFACTID(u8"HP"):
                  {
                      m_bo->updateHealth(std::lround(m_bo->m_sdHealth.getMaxHP() * percentage / 100.0) + value);
                      return {};
                  }
              case DBCOM_BUFFACTID(u8"MP"):
                  {
                      m_bo->updateHealth(0, std::lround(m_bo->m_sdHealth.getMaxMP() * percentage / 100.0) + value);
                      return {};
                  }
              case DBCOM_BUFFACTID(u8"HP上限"):
                  {
                      const auto addMaxHP = std::lround(m_bo->m_sdHealth.getMaxHP() * percentage / 100.0) + value;
                      const auto tag = m_bo->m_sdHealth.buffedMaxHP.add(addMaxHP);

                      m_bo->updateHealth();
                      return [tag, this]()
                      {
                          m_bo->m_sdHealth.buffedMaxHP.erase(tag);
                      };
                  }
              case DBCOM_BUFFACTID(u8"MP上限"):
                  {
                      const auto addMaxMP = std::lround(m_bo->m_sdHealth.getMaxMP() * percentage / 100.0) + value;
                      const auto tag = m_bo->m_sdHealth.buffedMaxMP.add(addMaxMP);

                      m_bo->updateHealth();
                      return [tag, this]()
                      {
                          m_bo->m_sdHealth.buffedMaxMP.erase(tag);
                      };
                  }
              case DBCOM_BUFFACTID(u8"HP持续"):
                  {
                      const auto addHP = std::lround(m_bo->m_sdHealth.getMaxHP() * percentage / 100.0) + value;
                      const auto tag = m_bo->m_sdHealth.buffedHPRecover.add(addHP);

                      m_bo->updateHealth();
                      return [tag, this]()
                      {
                          m_bo->m_sdHealth.buffedHPRecover.erase(tag);
                      };
                  }
              case DBCOM_BUFFACTID(u8"MP恢复"):
                  {
                      const auto addMP = std::lround(m_bo->m_sdHealth.getMaxMP() * percentage / 100.0) + value;
                      const auto tag = m_bo->m_sdHealth.buffedMPRecover.add(addMP);

                      m_bo->updateHealth();
                      return [tag, this]()
                      {
                          m_bo->m_sdHealth.buffedMPRecover.erase(tag);
                      };
                  }
              case DBCOM_BUFFACTID(u8"DC下限"):
              case DBCOM_BUFFACTID(u8"DC上限"):
                  {
                      const auto &[tag, taggedMap] = m_bo->updateBuffedAbility(buffActID(), percentage, value);
                      return [tag, &taggedMap]()
                      {
                          taggedMap.erase(tag);
                      };
                  }
              default:
                  {
                      throw fflvalue(getBR().name, getBAR().name, percentage, value);
                  }
          }
      }())
{}

BaseBuffActAttributeModifier::~BaseBuffActAttributeModifier()
{
    std::string errInfo;
    try{
        if(m_onDone){
            m_onDone();
        }
    }
    catch(const std::exception &e){
        errInfo = e.what();
    }
    catch(...){
        errInfo = "unknow error";
    }

    if(!errInfo.empty()){
        g_monoServer->addLog(LOGTYPE_FATAL, "Attribute modifier callback failed: %s", to_cstr(errInfo));
    }
}

BaseBuffActAttributeModifier *BaseBuffActAttributeModifier::createAttributeModifier(BattleObject *bo, uint32_t argBuffID, uint32_t argBuffActID)
{
    return new BaseBuffActAttributeModifier(bo, argBuffID, argBuffActID);
}

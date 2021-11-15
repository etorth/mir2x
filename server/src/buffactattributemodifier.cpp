#include "monoserver.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
#include "battleobject.hpp"
#include "buff.hpp"
#include "buffact.hpp"
#include "buffactattributemodifier.hpp"

extern MonoServer *g_monoServer;
BaseBuffActAttributeModifier::BaseBuffActAttributeModifier(BaseBuff *argBuff, size_t argBuffActOff)
    : BaseBuffAct(argBuff, argBuffActOff)
    , m_onDone([this]() -> std::function<void()>
      {
          fflassert(getBAR().isAttributeModifier());
          const auto percentage = getBAREF().attributeModifier.percentage;
          const auto value      = getBAREF().attributeModifier.value;

          fflassert(std::abs(percentage) >= 0);
          fflassert(std::abs(percentage) <= 100);

          const auto bo = getBuff()->getBO();
          switch(id()){
              case DBCOM_BUFFACTID(u8"HP"):
                  {
                      bo->updateHealth(std::lround(bo->m_sdHealth.getMaxHP() * percentage / 100.0) + value);
                      return {};
                  }
              case DBCOM_BUFFACTID(u8"MP"):
                  {
                      bo->updateHealth(0, std::lround(bo->m_sdHealth.getMaxMP() * percentage / 100.0) + value);
                      return {};
                  }
              case DBCOM_BUFFACTID(u8"HP上限"):
                  {
                      const auto addMaxHP = std::lround(bo->m_sdHealth.getMaxHP() * percentage / 100.0) + value;
                      const auto tag = bo->m_sdHealth.buffedMaxHP.add(addMaxHP);

                      bo->updateHealth();
                      return [tag, bo]()
                      {
                          bo->m_sdHealth.buffedMaxHP.erase(tag);
                      };
                  }
              case DBCOM_BUFFACTID(u8"MP上限"):
                  {
                      const auto addMaxMP = std::lround(bo->m_sdHealth.getMaxMP() * percentage / 100.0) + value;
                      const auto tag = bo->m_sdHealth.buffedMaxMP.add(addMaxMP);

                      bo->updateHealth();
                      return [tag, bo]()
                      {
                          bo->m_sdHealth.buffedMaxMP.erase(tag);
                      };
                  }
              case DBCOM_BUFFACTID(u8"HP持续"):
                  {
                      const auto addHP = std::lround(bo->m_sdHealth.getMaxHP() * percentage / 100.0) + value;
                      const auto tag = bo->m_sdHealth.buffedHPRecover.add(addHP);

                      bo->updateHealth();
                      return [tag, bo]()
                      {
                          bo->m_sdHealth.buffedHPRecover.erase(tag);
                      };
                  }
              case DBCOM_BUFFACTID(u8"MP恢复"):
                  {
                      const auto addMP = std::lround(bo->m_sdHealth.getMaxMP() * percentage / 100.0) + value;
                      const auto tag = bo->m_sdHealth.buffedMPRecover.add(addMP);

                      bo->updateHealth();
                      return [tag, bo]()
                      {
                          bo->m_sdHealth.buffedMPRecover.erase(tag);
                      };
                  }
              case DBCOM_BUFFACTID(u8"DC下限"):
              case DBCOM_BUFFACTID(u8"DC上限"):
              case DBCOM_BUFFACTID(u8"AC下限"):
              case DBCOM_BUFFACTID(u8"AC上限"):
              case DBCOM_BUFFACTID(u8"MAC下限"):
              case DBCOM_BUFFACTID(u8"MAC上限"):
                  {
                      const auto &[tag, taggedMap] = bo->updateBuffedAbility(id(), percentage, value);
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

BaseBuffActAttributeModifier *BaseBuffActAttributeModifier::createAttributeModifier(BaseBuff *argBuff, size_t argBuffActOff)
{
    return new BaseBuffActAttributeModifier(argBuff, argBuffActOff);
}

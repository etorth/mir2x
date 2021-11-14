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
          if(false
                  || getBAR().isBuffAct(u8"DC下限")
                  || getBAR().isBuffAct(u8"DC上限")){
              const auto &[tag, taggedMap] = m_bo->updateBuffedAbility(DBCOM_BUFFACTID(getBAR().name), 10);
              return [tag, &taggedMap]()
              {
                  taggedMap.erase(tag);
              };
          }

          if(getBAR().isBuffAct(u8"HP上限")){
              const auto tag = m_bo->m_sdHealth.buffedMaxHP.add(10);
              m_bo->updateHealth();
              return [tag, this]()
              {
                  m_bo->m_sdHealth.buffedMaxHP.erase(tag);
                  m_bo->updateHealth();
              };
          }

          if(getBAR().isBuffAct(u8"HP")){
              m_bo->updateHealth(10);
              return {};
          }

          if(getBAR().isBuffAct(u8"MP")){
              m_bo->updateHealth(0, 10);
              return {};
          }

          throw fflvalue(getBAR().name);
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

#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "basemagic.hpp"
#include "magicrecord.hpp"

static std::optional<uint32_t> magicGetSeffIDHelper(const std::u8string_view &name, const std::u8string_view &stage)
{
    fflassert(str_haschar(name));
    fflassert(str_haschar(stage));
    fflassert(magicStageID(stage.data()) >= MST_BEGIN);

    const auto [gfxEntry, gfxEntryRef] = DBCOM_MAGICGFXENTRY(name, stage);
    fflassert(gfxEntry, name, stage);

    if(gfxEntry->seff.ref){
        return magicGetSeffIDHelper(gfxEntry->seff.ref.name, gfxEntry->seff.ref.stage.empty() ? stage : gfxEntry->seff.ref.stage);
    }

    if(gfxEntry->seff.absSeffID.has_value()){
        return gfxEntry->seff.absSeffID;
    }

    const auto &mr = DBCOM_MAGICRECORD(name);
    fflassert(mr, name);

    if(!mr.seffBase.has_value()){
        return {};
    }

    return 0X04000000 + mr.seffBase.value() * 16 + (magicStageID(stage.data()) - MST_BEGIN);
}


BaseMagic::BaseMagic(const char8_t *magicName, const char8_t *magicStage, int dirIndex)
    : m_magicID([magicName]() -> uint32_t
      {
          if(const auto magicID = DBCOM_MAGICID(magicName)){
              return magicID;
          }
          throw fflerror("invalid magicName: %s", to_cstr(magicName));
      }())
    , m_magicRecord([this]() -> const auto &
      {
          if(const auto &mr = DBCOM_MAGICRECORD(magicID())){
              return mr;
          }
          throw fflerror("invalid magicID: %d", magicID());
      }())
    , m_gfxEntryPair([magicName, magicStage]()
      {
          fflassert(str_haschar(magicName));
          fflassert(str_haschar(magicStage));

          const auto gfxPair = DBCOM_MAGICGFXENTRY(magicName, magicStage);
          fflassert(gfxPair.first);
          fflassert(gfxPair.first->checkStage(magicStage));

          fflassert(gfxPair.first->frameCount > 0);
          fflassert(gfxPair.first->frameCount <= gfxPair.first->gfxIDCount);

          fflassert(gfxPair.first->speed >= SYS_MINSPEED);
          fflassert(gfxPair.first->speed <= SYS_MAXSPEED);
          return gfxPair;
      }())
    , m_seffID(magicGetSeffIDHelper(to_u8sv(magicName), to_u8sv(magicStage)))
    , m_gfxDirIndex(dirIndex)
{
    // gfxDirIndex is the index of gfx set
    // the gfx set can be for different direction or not
    fflassert(gfxDirIndex() >= 0 && gfxDirIndex() < getGfxEntry()->gfxDirType);
}

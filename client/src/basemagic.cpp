#include "magicdb.hpp"
#include "basemagic.hpp"

extern MagicDB *g_magicDB;
BaseMagic::BaseMagic(const char8_t *magicName, const char8_t *magicStage, int dirIndex)
    : m_magicID([magicName]() -> uint32_t
      {
          if(const auto magicID = DBCOM_MAGICID(magicName)){
              return magicID;
          }
          throw fflpanic("invalid magicName: {}", to_cstr(magicName));
      }())
    , m_magicRecord([this]() -> const auto &
      {
          if(const auto &mr = DBCOM_MAGICRECORD(magicID())){
              return mr;
          }
          throw fflpanic("invalid magicID: {}", magicID());
      }())
    , m_gfxEntryPair([magicName, magicStage]()
      {
          fflassert(str_haschar(magicName));
          fflassert(str_haschar(magicStage));

          const auto gfxPair = DBCOM_MAGICGFXENTRY(magicName, magicStage);
          fflassert(gfxPair.first);
          fflassert(gfxPair.first->checkStage(magicStage));

          fflassert(gfxPair.first->speed >= SYS_MINSPEED);
          fflassert(gfxPair.first->speed <= SYS_MAXSPEED);
          return gfxPair;
      }())
    , m_seffID(DBCOM_MAGICGFXSEFFID(to_u8sv(magicName), to_u8sv(magicStage)))
    , m_gfxDirIndex(dirIndex)
    , m_gfxFrameCount(g_magicDB->frameCount(m_gfxEntry, dirIndex))
{
    // gfxDirIndex is the index of gfx set
    // the gfx set can be for different direction or not
    fflassert(gfxDirIndex() >= 0, gfxDirIndex());
    fflassert(gfxDirIndex() < getGfxEntry()->gfxDirType, gfxDirIndex(), getGfxEntry()->gfxDirType);

    // entry without gfx can have no frames, its subclass decides when it's done
    fflassert(gfxFrameCount() > 0 || (m_gfxEntry->gfxID == SYS_U32NIL && !m_gfxEntry->loop), to_cstr(magicName), to_cstr(magicStage), gfxFrameCount());
}

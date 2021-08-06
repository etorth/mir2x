#include "strf.hpp"
#include "hero.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "pngtexoffdb.hpp"
#include "motioneffect.hpp"
#include "clientcreature.hpp"

extern PNGTexOffDB *g_magicDB;
extern SDLDevice *g_sdlDevice;

MotionEffect::MotionEffect(const char8_t *magicName, const char8_t *stageName, MotionNode *motionPtr)
    : m_accuTime(0.0)
    , m_gfxEntry([magicName, stageName]()
      {
          fflassert(str_haschar(magicName));
          fflassert(str_haschar(stageName));

          const auto &mr = DBCOM_MAGICRECORD(magicName);
          fflassert(mr);

          const auto &ge = mr.getGfxEntry(stageName);
          fflassert(ge);
          fflassert(ge.loop == 0);
          fflassert(ge.checkType(u8"附着"));

          fflassert(ge.frameCount > 0);
          fflassert(ge.frameCount <= ge.gfxIDCount);

          fflassert(ge.speed >= SYS_MINSPEED);
          fflassert(ge.speed <= SYS_MAXSPEED);
          return &ge;
      }())
    , m_motion(motionPtr)
{
    fflassert(m_motion);
    fflassert(directionValid(m_motion->direction));
}


uint32_t MotionEffect::frameTexID() const
{
    if(m_gfxEntry->gfxDirType > 1){
        return m_gfxEntry->gfxID + frame() + (m_motion->direction - DIR_BEGIN) * m_gfxEntry->gfxIDCount;
    }
    return m_gfxEntry->gfxID + frame();
}

void MotionEffect::drawShift(int shiftX, int shiftY, uint32_t modColor)
{
    if(const auto texID = frameTexID(); texID != SYS_TEXNIL){
        if(auto [texPtr, offX, offY] = g_magicDB->retrieve(texID); texPtr){
            SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, modColor);
            SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
            g_sdlDevice->drawTexture(texPtr, shiftX + offX, shiftY + offY);
        }
    }
}

HeroSpellMagicEffect::HeroSpellMagicEffect(const char8_t *magicName, Hero *heroPtr, MotionNode *motionPtr)
    : MotionEffect(magicName, u8"启动", motionPtr)
    , m_hero(heroPtr)
{
    fflassert(m_hero);
    switch(m_motion->type){
        case MOTION_SPELL0:
        case MOTION_SPELL1: break;
        default           : throw fflerror("invalid motion type: %s", motionName(m_motion->type));
    }

    switch(m_gfxEntry->gfxDirType){
        case  1: break;
        case  8: break;
        default: throw fflerror("invalid gfxDirType: %d", m_gfxEntry->gfxDirType);
    }
}

int HeroSpellMagicEffect::frameCount() const
{
    switch(m_motion->type){
        case MOTION_SPELL0: return std::max<int>(MotionEffect::frameCount(),  8);
        case MOTION_SPELL1: return std::max<int>(MotionEffect::frameCount(), 10);
        default           : throw fflerror("invalid motion type: %s", motionName(m_motion->type));
    }
}

uint32_t HeroSpellMagicEffect::frameTexID() const
{
    if(frame() < MotionEffect::frameCount()){
        return MotionEffect::frameTexID();
    }
    return SYS_TEXNIL;
}

void HeroSpellMagicEffect::update(double ms)
{
    m_accuTime += ms;
    if(m_hero->checkUpdate(ms)){
        const int effectEndFrame = frameCount() - 1;
        const int motionEndFrame = m_hero->getFrameCountEx(m_motion->type, m_motion->direction) - 1;
        const int motionSyncFrameCount = [this]() -> int
        {
            if(m_motion->type == MOTION_SPELL0){
                return 3;
            }
            return 1;
        }();
        const int effectSyncFrameCount = [motionSyncFrameCount, this]() -> int
        {
            return std::lround((to_df(speed()) * motionSyncFrameCount) / m_motion->speed);
        }();

        if( m_motion->frame >= motionEndFrame - motionSyncFrameCount && frame() < effectEndFrame - effectSyncFrameCount){
            m_motion->frame  = motionEndFrame - motionSyncFrameCount;
        }
        else{
            m_hero->updateMotion();
        }
    }
}

MotionSyncEffect::MotionSyncEffect(const char8_t *magicName, const char8_t *stageName, ClientCreature *creaturePtr, MotionNode *motionPtr)
    : MotionEffect(magicName, stageName, motionPtr)
    , m_creature(creaturePtr)
{}

int MotionSyncEffect::absFrame() const
{
    const auto fspeed = [this]() -> double
    {
        if(m_useMotionSpeed){
            return to_df(m_motion->speed * m_gfxEntry->frameCount) / to_df(m_creature->getFrameCountEx(m_motion->type, m_motion->direction));
        }
        return to_df(m_gfxEntry->speed);
    }();
    return std::lround((m_accuTime / 1000.0) * SYS_DEFFPS * (fspeed / 100.0));
}

void MotionSyncEffect::update(double ms)
{
    m_accuTime += ms;
    if(m_creature->checkUpdate(ms)){
        m_creature->updateMotion(); // this can deallocate m_motion
    }
}

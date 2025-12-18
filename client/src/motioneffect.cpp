#include "strf.hpp"
#include "hero.hpp"
#include "pathf.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "pngtexoffdb.hpp"
#include "motioneffect.hpp"
#include "clientcreature.hpp"

extern PNGTexOffDB *g_magicDB;
extern SDLDevice *g_sdlDevice;

MotionEffect::MotionEffect(const char8_t *magicName, const char8_t *stageName, MotionNode *motionPtr)
    : m_accuTime(0.0)
    , m_motion(motionPtr)
    , m_gfxEntryPair([magicName, stageName]()
      {
          fflassert(str_haschar(magicName));
          fflassert(str_haschar(stageName));

          const auto gfxPair = DBCOM_MAGICGFXENTRY(magicName, stageName);
          fflassert(gfxPair.first);
          fflassert(gfxPair.first->loop == 0);
          fflassert(gfxPair.first->checkType(u8"附着"));

          fflassert(gfxPair.first->frameCount > 0);
          fflassert(gfxPair.first->frameCount <= gfxPair.first->gfxIDCount);

          fflassert(gfxPair.first->speed >= SYS_MINSPEED);
          fflassert(gfxPair.first->speed <= SYS_MAXSPEED);
          return gfxPair;
      }())
{
    fflassert(m_motion);
    fflassert(pathf::dirValid(m_motion->direction));
}

uint32_t MotionEffect::frameTexID() const
{
    fflassert(!done());
    if(const auto gfxIndex = gfxFrame(); gfxIndex >= 0){
        if(m_gfxEntry->gfxDirType > 1){
            return m_gfxEntry->gfxID + gfxIndex + (m_motion->direction - DIR_BEGIN) * m_gfxEntry->gfxIDCount;
        }
        return m_gfxEntry->gfxID + gfxIndex;
    }
    return SYS_U32NIL;
}

void MotionEffect::drawShift(int shiftX, int shiftY, uint32_t modColor)
{
    // m_motion is always valid in this function
    // because we call it by currMotion: CO::m_currMotion->effect->drawShift()

    fflassert(!done());
    if(const auto texID = frameTexID(); texID != SYS_U32NIL){
        if(auto [texPtr, offX, offY] = g_magicDB->retrieve(texID); texPtr){
            SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::modRGBA(m_gfxEntryRef ? m_gfxEntryRef->modColor : m_gfxEntry->modColor, modColor));
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
        case MOTION_SPELL1:
        case MOTION_ATTACKMODE: break;
        default: throw fflerror("invalid motion type: %s", motionName(m_motion->type));
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
        case MOTION_SPELL0: return std::max<int>(MotionEffect::frameCount(), 8);
        case MOTION_SPELL1:
        case MOTION_ATTACKMODE: return std::max<int>(MotionEffect::frameCount(), 10);
        default: throw fflerror("invalid motion type: %s", motionName(m_motion->type));
    }
}

uint32_t HeroSpellMagicEffect::frameTexID() const
{
    if(frame() < MotionEffect::frameCount()){
        return MotionEffect::frameTexID();
    }
    return SYS_U32NIL;
}

void HeroSpellMagicEffect::update(double ms)
{
    fflassert(!done());
    m_accuTime += ms;

    if(m_hero->checkUpdate(ms)){
        const int effectEndFrame = frameCount() - 1;
        const int motionEndFrame = m_hero->getFrameCountEx(m_motion) - 1;
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
            m_hero->updateMotion(false);
        }
    }
}

MotionAlignedEffect::MotionAlignedEffect(const char8_t *magicName, const char8_t *stageName, ClientCreature *creaturePtr, MotionNode *motionPtr, bool useMotionSpeed)
    : MotionEffect(magicName, stageName, motionPtr)
    , m_creature(creaturePtr)
    , m_useMotionSpeed(useMotionSpeed)
{}

int MotionAlignedEffect::absFrame() const
{
    const auto fspeed = [this]() -> double
    {
        if(m_useMotionSpeed){
            return to_df(m_motion->speed * m_gfxEntry->frameCount) / to_df(m_creature->getFrameCountEx(m_motion));
        }
        return to_df(m_gfxEntry->speed);
    }();
    return std::lround((m_accuTime / 1000.0) * SYS_DEFFPS * (fspeed / 100.0));
}

void MotionAlignedEffect::update(double ms)
{
    fflassert(!done());
    m_accuTime += ms;

    if(m_creature->checkUpdate(ms)){
        m_creature->updateMotion(false);
    }
}

MotionSyncEffect::MotionSyncEffect(const char8_t *magicName, const char8_t *stageName, ClientCreature *creaturePtr, MotionNode *motionPtr, int lagFrame)
    : MotionEffect(magicName, stageName, motionPtr)
    , m_creature(creaturePtr)
    , m_lagFrame(lagFrame)
{}

int MotionSyncEffect::speed() const
{
    // previously redefined:
    //
    //     MotionSyncEffect::absFrame() const
    //     {
    //         return m_motion->frame;
    //     }
    //
    // the problem is m_motion->frame never go above frameCount
    // this makes MotionSyncEffect::done() never returns true, then the ClientCreature::update() gets stuck

    return m_motion->speed;
}

int MotionSyncEffect::gfxFrame() const
{
    return absFrame() - m_lagFrame;
}

int MotionSyncEffect::frameCount() const
{
    return std::min<int>(MotionEffect::frameCount() + m_lagFrame, m_creature->getFrameCountEx(m_motion));
}

void MotionSyncEffect::update(double ms)
{
    fflassert(!done());
    m_accuTime += ms;

    if(m_creature->checkUpdate(ms)){
        m_creature->updateMotion(false);
    }
}

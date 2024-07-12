#include <algorithm>
#include <tinyxml2.h>
#include <SDL2/SDL.h>

#include "log.hpp"
#include "mathf.hpp"
#include "motion.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "sdldevice.hpp"
#include "ascendstr.hpp"
#include "processrun.hpp"
#include "magicrecord.hpp"
#include "protocoldef.hpp"
#include "attachmagic.hpp"
#include "soundeffectdb.hpp"
#include "clientcreature.hpp"
#include "clientmonster.hpp"

extern Log *g_log;
extern SDLDevice *g_sdlDevice;
extern SoundEffectDB *g_seffDB;

bool ClientCreature::advanceMotionFrame()
{
    m_currMotion->frame = (m_currMotion->frame + 1) % getFrameCountEx(m_currMotion.get());
    return true;
}

void ClientCreature::updateAttachMagic(double fUpdateTime)
{
    for(auto p = m_attachMagicList.begin(); p != m_attachMagicList.end();){
        if((*p)->update(fUpdateTime)){
            p = m_attachMagicList.erase(p);
        }
        else{
            ++p;
        }
    }
}

void ClientCreature::setBuffIDList(SDBuffIDList list)
{
    m_sdBuffIDList = std::move(list);
}

void ClientCreature::updateHealth(SDHealth health)
{
    const int pixelX = x() * SYS_MAPGRIDXP + SYS_MAPGRIDXP / 2;
    const int pixelY = y() * SYS_MAPGRIDYP - SYS_MAPGRIDYP * 1;

    if(!m_sdHealth.has_value()){
        m_sdHealth = std::move(health);
        return;
    }

    const int diffHP = health.hp - m_sdHealth.value().hp;
    const int diffMP = health.mp - m_sdHealth.value().mp;

    m_sdHealth = health;
    if(diffHP > 0){
        m_processRun->addAscendStr(ASCENDSTR_GREEN, diffHP, pixelX, pixelY);
    }
    else if(diffHP < 0){
        m_processRun->addAscendStr(ASCENDSTR_RED, diffHP, pixelX, pixelY);
    }

    if(diffMP > 0){
        m_processRun->addAscendStr(ASCENDSTR_BLUE, diffMP, pixelX, pixelY - (diffHP ? SYS_MAPGRIDYP : 0));
    }
}

bool ClientCreature::alive() const
{
    if(!motionValid(m_currMotion)){
        throw fflerror("invalid motion detected");
    }

    switch(m_currMotion->type){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                return false;
            }
        default:
            {
                return true;
            }
    }
}

bool ClientCreature::active() const
{
    if(!motionValid(m_currMotion)){
        throw fflerror("invalid motion detected");
    }

    switch(m_currMotion->type){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                return (m_currMotion->frame + 1) < getFrameCountEx(m_currMotion.get());
            }
        default:
            {
                return true;
            }
    }
}

bool ClientCreature::visible() const
{
    fflassert(motionValid(m_currMotion));
    switch(m_currMotion->type){
        case MOTION_DIE:
            {
                fflassert(type() == UID_PLY);
                return ((m_currMotion->frame + 1) < getFrameCountEx(m_currMotion.get())) || (m_currMotion->extParam.die.fadeOut < 255);
            }
        case MOTION_MON_DIE:
            {
                // NOTE check (frame + 1) < frameCount
                // because ClientCreature::update() guarantees frame < frmeCount always true

                fflassert(type() == UID_MON);
                if((m_currMotion->frame + 1) < getFrameCountEx(m_currMotion.get())){
                    return true;
                }

                const auto monCPtr = dynamic_cast<const ClientMonster *>(this);
                if(monCPtr->getMR().deadFadeOut){
                    return m_currMotion->extParam.die.fadeOut < 255;
                }

                fflassert(m_currMotion->extParam.die.fadeOut == 0);
                return false;
            }
        default:
            {
                return true;
            }
    }
}

bool ClientCreature::checkUpdate(double ms)
{
    fflassert(ms >= 0.0);
    m_accuUpdateTime += ms;

    if(m_accuUpdateTime >= m_lastUpdateTime + m_currMotion->frameDelay()){
        m_lastUpdateTime = m_accuUpdateTime;
        return true;
    }
    return false;
}

void ClientCreature::querySelf()
{
    m_lastQuerySelf = SDL_GetTicks();
    m_processRun->queryCORecord(UID());
}

bool ClientCreature::isMonster(const char8_t *name) const
{
    return (type() == UID_MON) && (dynamic_cast<const ClientMonster *>(this)->monsterID() == DBCOM_MONSTERID(name));
}

void ClientCreature::setBuff(int, int)
{
    // do nothing by default
}

void ClientCreature::playSoundEffect(uint32_t seffID)
{
    if(seffID == SYS_U32NIL){
        return;
    }

    fflassert(m_processRun);
    fflassert(m_processRun->getMyHero());
    m_processRun->playSoundEffectAt(seffID, x(), y(), 1);
}

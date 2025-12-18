#include "fflerror.hpp"
#include "dbcomid.hpp"
#include "skillboardconfig.hpp"
#include "skillboard.hpp"

std::optional<char> SkillBoardConfig::getMagicKey(uint32_t magicID) const
{
    if(auto p = m_learnedMagicList.find(magicID); p != m_learnedMagicList.end()){
        return p->second.key;
    }
    return {};
}

std::optional<int> SkillBoardConfig::getMagicLevel(uint32_t magicID) const
{
    if(auto p = m_learnedMagicList.find(magicID); p != m_learnedMagicList.end()){
        return p->second.level;
    }
    return {};
}

void SkillBoardConfig::setMagicLevel(uint32_t magicID, int level)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert(SkillBoard::getMagicIconGfx(magicID));

    fflassert(level >= 1);
    fflassert(level <= 3);

    if(auto p = m_learnedMagicList.find(magicID); p != m_learnedMagicList.end()){
        fflassert(level >= p->second.level);
        p->second.level = level;
    }
    else{
        m_learnedMagicList[magicID].level = level;
    }
}

void SkillBoardConfig::setMagicKey(uint32_t magicID, std::optional<char> key)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert(SkillBoard::getMagicIconGfx(magicID));

    fflassert(hasMagicID(magicID));
    fflassert(!SkillBoard::getMagicIconGfx(magicID)->passive);
    fflassert(!key.has_value() || (key.value() >= 'a' && key.value() <= 'z') || (key.value() >= '0' && key.value() <= '9'));

    m_learnedMagicList[magicID].key = key;
    if(key.has_value()){
        for(auto &p: m_learnedMagicList){
            if((p.first != magicID) && p.second.key == key){
                p.second.key.reset();
            }
        }
    }
}

#pragma once
#include <list>
#include <cmath>
#include <cstdint>
#include <functional>
#include "totype.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "dbcomrecord.hpp"
#include "magicrecord.hpp"

class BaseMagic
{
    protected:
        const uint32_t m_magicID;
        const MagicRecord &m_magicRecord;

    private:
        const std::pair<const MagicGfxEntry &, const MagicGfxEntryRef &> m_gfxEntryPair;

    protected:
        const MagicGfxEntry    &m_gfxEntry    = m_gfxEntryPair.first;
        const MagicGfxEntryRef &m_gfxEntryRef = m_gfxEntryPair.second;

    protected:
        const int m_gfxDirIndex;

    protected:
        double m_accuTime = 0.0;

    protected:
        std::list<std::function<void(BaseMagic *)>> m_onDoneCBList;
        std::list<std::function<bool(BaseMagic *)>> m_onUpdateCBList;

    public:
        BaseMagic(const char8_t *magicName, const char8_t *magicStage, int dirIndex)
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
            , m_gfxEntryPair([magicStage, this]()
              {
                  if(const auto &p = m_magicRecord.getGfxEntry(magicStage); p.first){
                      return p;
                  }
                  throw fflerror("invalid magicStage: %s", to_cstr(magicStage));
              }())
            , m_gfxDirIndex(dirIndex)
        {
            // gfxDirIndex is the index of gfx set
            // the gfx set can be for different direction or not
            fflassert(gfxDirIndex() >= 0 && gfxDirIndex() < getGfxEntry().gfxDirType);
        }

    public:
        virtual ~BaseMagic() = default;

    public:
        uint32_t magicID() const
        {
            return m_magicID;
        }

        const char8_t *magicName() const
        {
            return DBCOM_MAGICRECORD(magicID()).name;
        }

        const MagicGfxEntry &getGfxEntry() const
        {
            return m_gfxEntry;
        }

        int gfxDirIndex() const
        {
            return m_gfxDirIndex;
        }

    public:
        bool checkMagic(const char8_t *name) const
        {
            return magicID() == DBCOM_MAGICID(name);
        }

        bool checkMagic(const char8_t *name, const char8_t *stage) const
        {
            return magicID() == DBCOM_MAGICID(name) && getGfxEntry().checkStage(stage);
        }

    public:
        // why return bool
        // we usually use following code to update magic
        //
        // 1   for(auto p = magicList.begin(); p != magicList.end()){
        // 2       p->update(fUpdateTime);
        // 3       if(p->done()){
        // 4           p = magicList.erase(p);
        // 5       }
        // 6       else{
        // 7           ++p;
        // 8       }
        // 9   }
        //
        // this function has one issue:
        // line 2 inside update() the done() returns false, but line 3 it can returns true, especially when we use timing to calculate current frame index
        // this causes we missed to call the onDone callback
        // instead use following code:
        //
        // 1   for(auto p = magicList.begin(); p != magicList.end()){
        // 3       if(p->update(fUpdateTime)){
        // 4           p = magicList.erase(p);
        // 5       }
        // 6       else{
        // 7           ++p;
        // 8       }
        // 9   }
        //
        // we mark done() as protected, but when use magic composition I have to keep it public
        // user shouldn't call it directly

        virtual bool update(double ms)
        {
            m_accuTime += ms;
            runOnUpdate();

            if(done()){
                runOnDone();
                return true;
            }
            return false;
        }

    public:
        int frame() const
        {
            if(m_gfxEntry.loop){
                return absFrame() % m_gfxEntry.frameCount;
            }
            else{
                return absFrame();
            }
        }

        virtual int absFrame() const
        {
            return std::lround((m_accuTime / 1000.0) * SYS_DEFFPS * (m_gfxEntry.speed / 100.0));
        }

    public:
        void addOnDone(std::function<void(BaseMagic *)> onDone)
        {
            m_onDoneCBList.push_back(std::move(onDone));
        }

        void addTrigger(std::function<bool(BaseMagic *)> onUpdate)
        {
            m_onUpdateCBList.push_back(std::move(onUpdate));
        }

    public:
        virtual bool done() const
        {
            if(m_gfxEntry.loop){
                return false;
            }
            return absFrame() >= m_gfxEntry.frameCount;
        }

    protected:
        void runOnDone()
        {
            fflassert(done());
            for(auto &f: m_onDoneCBList){
                f(this);
            }
            m_onDoneCBList.clear();
        }

        void runOnUpdate()
        {
            for(auto p = m_onUpdateCBList.begin(); p != m_onUpdateCBList.end();){
                if((*p)(this)){
                    p = m_onUpdateCBList.erase(p);
                }
                else{
                    ++p;
                }
            }
        }
};

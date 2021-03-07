/*
 * =====================================================================================
 *
 *       Filename: magicbase.hpp
 *        Created: 08/05/2017 22:58:20
 *    Description: base of AttachMagic and IndepMagic
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <list>
#include <cstdint>
#include <functional>
#include "totype.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
#include "magicrecord.hpp"

class MagicBase
{
    protected:
        const uint32_t m_magicID;
        const MagicRecord &m_magicRecord;

    protected:
        const MagicGfxEntry &m_gfxEntry;

    protected:
        const int m_gfxDirIndex;

    protected:
        double m_accuTime = 0.0;

    protected:
        std::list<std::function<void(MagicBase *)>> m_onDoneCBList;
        std::list<std::function<bool(MagicBase *)>> m_onUpdateCBList;

    public:
        MagicBase(const char8_t *magicName, const char8_t *magicStage, int gfxDirIndex)
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
            , m_gfxEntry([magicStage, this]() -> const auto &
              {
                  if(const auto &ge = m_magicRecord.getGfxEntry(magicStage)){
                      return ge;
                  }
                  throw fflerror("invalid magicStage: %s", to_cstr(magicStage));
              }())
            , m_gfxDirIndex(gfxDirIndex)
        {
            switch(m_gfxEntry.gfxDirType){
                case  4:
                case  8:
                case 16:
                    {
                        if(!(gfxDirIndex >= 0 && gfxDirIndex < m_gfxEntry.gfxDirType)){
                            throw fflerror("invalid gfx direction index: %d", m_gfxDirIndex);
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }

    public:
        virtual ~MagicBase() = default;

    public:
        uint32_t magicID() const
        {
            return m_magicID;
        }

        const char8_t *magicName() const
        {
            return DBCOM_MAGICRECORD(magicID()).name;
        }

        const auto &getGfxEntry() const
        {
            return m_gfxEntry;
        }

        int gfxDirIndex() const
        {
            return m_gfxDirIndex;
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
        // we mark done() as protected
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
        bool stageDone() const
        {
            if(m_gfxEntry.loop){
                return false;
            }
            return absFrame() >= m_gfxEntry.frameCount;
        }

        int frame() const
        {
            switch(m_gfxEntry.loop){
                case  0: return absFrame();
                case  1: return absFrame() % m_gfxEntry.frameCount;
                default: return -1;
            }
        }

        int absFrame() const
        {
            return (m_accuTime / 1000.0) * SYS_DEFFPS * (m_gfxEntry.speed / 100.0);
        }

    public:
        void addOnDone(std::function<void(MagicBase *)> onDone)
        {
            m_onDoneCBList.push_back(std::move(onDone));
        }

        void addOnUpdate(std::function<bool(MagicBase *)> onUpdate)
        {
            m_onUpdateCBList.push_back(std::move(onUpdate));
        }

    protected:
        virtual bool done() const
        {
            return stageDone();
        }

    protected:
        void runOnDone()
        {
            if(!done()){
                throw fflerror("magic is not done yet");
            }

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

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
#include <functional>
#include "totype.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
#include "magicrecord.hpp"

class MagicBase
{
    protected:
        const int m_magicID;
        const MagicRecord *m_magicRecord;

    protected:
        const MagicGfxEntry *m_gfxEntry;

    protected:
        double m_accuTime = 0.0;

    protected:
        int m_gfxDirIndex;

    protected:
        std::list<std::function<bool()>> m_onUpdateList;

    protected:
        bool m_onDoneCalled = false;
        std::list<std::function<void()>> m_onDoneList;

    public:
        MagicBase(const char8_t *magicName, const char8_t *magicStage, int gfxDirIndex)
            : m_magicID([magicName]()
              {
                  if(const auto magicID = DBCOM_MAGICID(magicName)){
                      return magicID;
                  }
                  throw fflerror("invalid magicName: %s", to_cstr(magicName));
              }())
            , m_magicRecord([this]()
              {
                  if(const auto &mr = DBCOM_MAGICRECORD(magicID())){
                      return &mr;
                  }
                  throw fflerror("invalid magicID: %d", magicID());
              }())
            , m_gfxEntry([magicStage, this]()
              {
                  if(const auto &ge = m_magicRecord->getGfxEntry(magicStage)){
                      return &ge;
                  }
                  throw fflerror("invalid magicStage: %s", to_cstr(magicStage));
              }())
            , m_gfxDirIndex(gfxDirIndex)
        {
            switch(m_gfxEntry->gfxDirType){
                case  4:
                case  8:
                case 16:
                    {
                        if(!(gfxDirIndex >= 0 && gfxDirIndex < m_gfxEntry->gfxDirType)){
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
        virtual ~MagicBase()
        {
            if(!m_onDoneCalled){
                for(auto &f: m_onDoneList){
                    f();
                }
            }
        }

    public:
        int magicID() const
        {
            return m_magicID;
        }

        const char8_t *magicName() const
        {
            return DBCOM_MAGICRECORD(magicID()).name;
        }

        const auto &getGfxEntry() const
        {
            return m_gfxEntry[0];
        }

        int gfxDirIndex() const
        {
            return m_gfxDirIndex;
        }

    public:
        virtual void update(double ms)
        {
            m_accuTime += ms;
            runOnUpdate();

            if(done()){
                runOnDone();
            }
        }

    public:
        bool stageDone() const
        {
            if(m_gfxEntry->loop){
                return false;
            }
            return absFrame() >= m_gfxEntry->frameCount;
        }

        int frame() const
        {
            switch(m_gfxEntry->loop){
                case  0: return absFrame();
                case  1: return absFrame() % m_gfxEntry->frameCount;
                default: return -1;
            }
        }

        int absFrame() const
        {
            return (m_accuTime / 1000.0) * SYS_DEFFPS * (m_gfxEntry->speed / 100.0);
        }

    public:
        void addOnDone(std::function<void()> onDone)
        {
            m_onDoneList.push_back(std::move(onDone));
        }

        void addOnUpdate(std::function<bool()> onUpdate)
        {
            m_onUpdateList.push_back(std::move(onUpdate));
        }

    public:
        virtual bool done() const
        {
            return stageDone();
        }

    protected:
        void runOnDone()
        {
            if(!done()){
                throw fflerror("call runOnDone() while magic is not done yet");
            }

            if(m_onDoneCalled){
                throw fflerror("call runOnDone() twice");
            }

            for(auto &f: m_onDoneList){
                f();
            }
            m_onDoneCalled = true;
        }

        void runOnUpdate()
        {
            for(auto p = m_onUpdateList.begin(); p != m_onUpdateList.end();){
                if((*p)()){
                    p = m_onUpdateList.erase(p);
                }
                else{
                    ++p;
                }
            }
        }
};

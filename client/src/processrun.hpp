/*
 * =====================================================================================
 *
 *       Filename: processrun.hpp
 *        Created: 08/31/2015 03:42:07 AM
 *  Last Modified: 06/02/2016 22:56:21
 *
 *    Description: 
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
#include <cstdint>
#include <unordered_map>

#include "process.hpp"
#include "message.hpp"
#include "mir2xmap.hpp"
#include "creature.hpp"
#include "clientmap.hpp"

class ProcessRun: public Process
{
    private:
        ClientMap   m_ClientMap;
        int         m_ViewX;
        int         m_ViewY;

    private:
        std::unordered_map<uint64_t, Creature*> m_CreatureMap;

    public:
        ProcessRun();
        virtual ~ProcessRun() = default;

    public:
        virtual int ID()
        {
            return PROCESSID_RUN;
        }

    public:
        virtual void Update(double);
        virtual void Draw();
        virtual void ProcessEvent(const SDL_Event &);

    public:
        void Net_LoginOK(const uint8_t *, size_t);
        void Net_MotionState(const uint8_t *, size_t);
        void Net_MONSTERGINFO(const uint8_t *, size_t);
};

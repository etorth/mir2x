/*
 * =====================================================================================
 *
 *       Filename: processrun.hpp
 *        Created: 8/31/2015 3:42:07 AM
 *  Last Modified: 09/09/2015 2:26:13 AM
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
#include <list>
#include <mutex>
#include "hero.hpp"
#include "myhero.hpp"
#include "monster.hpp"
#include "process.hpp"
#include "message.hpp"
#include "mir2clientmap.hpp"
#include "clientmessagedef.hpp"

class ProcessRun: public Process
{
    private:
        Mir2ClientMap   m_Map;

    public:
        ProcessRun(Game *);
        ~ProcessRun();

    public:
        void Enter();
        void Exit();
        void Update();
        void Draw();
        void HandleEvent(SDL_Event *);

    public:
        void LoadMap(const char *);

    private:
        void RollScreen();

    private:
        void HandleMessage(const Message &);

    private:
        void DoLogin();

    private:
        void UpdateActor();
        void UpdateMyHero();

    public:
        void OnLoginSucceed(const ClientMessageLoginSucceed &);

    public:
        Actor *NewActor(int, int, int);

    private:
        std::list<Actor *> m_ActorList;
        std::mutex         m_ActorListMutex;
        MyHero            *m_MyHero;
        std::mutex         m_MyHeroMutex;

    private:
        int m_WindowW;
        int m_WindowH;
};

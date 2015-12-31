/*
 * =====================================================================================
 *
 *       Filename: actor.hpp
 *        Created: 9/3/2015 5:48:38 PM
 *  Last Modified: 09/09/2015 6:29:01 PM
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

#include <tuple>
#include <vector>
#include <cstdint>
#include "newmir2map.hpp"
#include "actorstatedef.hpp"
#include "directiverectcover.hpp"

class Actor
{
    public:
        Actor(int, int, int);
        ~Actor();

    public:
        int UID()       const;
        int SID()       const;
        int GenTime()   const;
        int State()     const;
        int Direction() const;
        int HP()        const;
        int Y()         const;
        int X()         const;

    protected:
        NewMir2Map *m_Map;

    protected:
        int m_SID;
        int m_UID;
        int m_GenTime;
        int m_State;
        int m_Direction;
        int m_X;
        int m_Y;
        int m_HP;
        int m_DRCIndex;

    protected:
        int m_LastSyrcTime;

    protected:
        bool SetDRCIndex();

    public:
        void SetDirection(int);

    public:
        virtual void Update() = 0;

    public:
        virtual void SetMap(int, int, NewMir2Map *);
        virtual bool Collide(const Actor *);
        virtual bool RandomStart();
        virtual void BroadcastBaseInfo();

    public:
        const DirectiveRectCover &DRCover() const;

        // global cover information
    protected:
        static std::vector<std::tuple<int, int, DirectiveRectCover>> m_SIDStateDRCTupleV;

    public:
        static bool GlobalCoverInfoValid(int);
        static bool SetGlobalCoverInfo(int, int, int, int, int);
        static int  GlobalCoverInfoIndex(int, int, int);
};

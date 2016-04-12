/*
 * =====================================================================================
 *
 *       Filename: serverobject.hpp
 *        Created: 04/11/2016 19:54:41
 *  Last Modified: 04/11/2016 21:00:47
 *
 *    Description: object with Type()/Mode()/State()
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

#include "asyncobject.hpp"

// this design pattern is not perfect, when we define valid state/type/mode for class A, and
// then we define class B inherient from A, then B may have new state/type/mode available, 
// what we should do? if we take this pattern, we need to list all state/type/mode here for
// class A and all possible derived class B, C, D....
//
// since I take charge of all these source code file and I can edit it, it's OK, I like its
// simplified interface.

class ServerObject: public AsyncObject
{
    protected:
        uint32_t m_UID;
        uint32_t m_AddTime;

    public:
        uint32_t UID()
        {
            return m_UID;
        }

        uint32_t AddTime()
        {
            return m_AddTime;
        }

    public:
        // getter
        virtual bool Mode (uint8_t) = 0;
        virtual bool Type (uint8_t) = 0;
        virtual bool State(uint8_t) = 0;

        // setter
        virtual void SetMode (uint8_t, bool) = 0;
        virtual void SetType (uint8_t, bool) = 0;
        virtual void SetState(uint8_t, bool) = 0;
};

enum ModeType: uint8_t{
    MODE_NEVERDIE,
    MODE_ATTACKALL,
    MODE_PEACE,
};

enum ObjectType: uint8_t{
    OBJECT_CHAROBJECT,
    OGJECT_ITEM,
    OBJECT_EVENT,

    OBJECT_HUMAN,
    OBJECT_PLAYER,
    OBJECT_NPC,
    OBJECT_ANIMAL,
    OBJECT_MONSTER,
};

enum ObjectState: uint8_t{
    STATE_MOVING,
    STATE_DEAD,
    STATE_GHOST,
};


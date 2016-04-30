/*
 * =====================================================================================
 *
 *       Filename: activeobject.hpp
 *        Created: 04/11/2016 19:54:41
 *  Last Modified: 04/29/2016 18:13:37
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
#include <array>
#include <cstdint>

#include "reactobject.hpp"
#include "serverobject.hpp"

// this design pattern is not perfect, when we define valid state/type/mode for class A, and
// then we define class B inherient from A, then B may have new state/type/mode available, 
// what we should do? if we take this pattern, we need to list all state/type/mode here for
// class A and all possible derived class B, C, D....
//
// since I take charge of all these source code file and I can edit it, it's OK, I like its
// simplified interface.
//
// Let me make State and Type only, and merge Mode into State for less interfaces
//
// TODO: define life circle of an active object:
//
//  1. STATE_EMBRYO
//  2. STATE_INCARNATED
//  3. STATE_PHANTOM
//
//

enum ObjectType: uint8_t{
    OBJECT_HUMAN,
    OBJECT_PLAYER,
    OBJECT_NPC,
    OBJECT_ANIMAL,
    OBJECT_MONSTER,
};

enum ObjectState: uint8_t{
    // three states of an active object
    STATE_EMBRYO,
    STATE_INCARNATED,
    STATE_PHANTOM,

    STATE_MOVING,
    STATE_DEAD,


    STATE_NEVERDIE,
    STATE_ATTACKALL,
    STATE_PEACE,
    STATE_CANMOVE,
    STATE_WAITMOVE,
};

class ActiveObject: public ReactObject
{
    protected:
        std::array<    bool, 255> m_TypeV;
        std::array<    bool, 255> m_StateV;
        std::array<uint32_t, 255> m_StateTimeV;

    public:
        ActiveObject(uint32_t nUID, uint32_t nAddTime)
            : ReactObject(CATEGORY_ACTIVEOBJECT, nUID, nAddTime)
        {
            m_TypeV.fill(0);
            m_StateV.fill(0);
            m_StateTimeV.fill(0);

            m_StateV[STATE_EMBRYO] = 1;
        }

    public:
        // getter
        // always return values, nonthrow
        //
        //   true: the object is in state/type as indicated
        //  false: the object is not in passed state/type or the passed
        //               state/type is not supported
        //
        // when using this getter function, only return true makes sense
        //
        virtual bool Type (uint8_t) = 0;
        virtual bool State(uint8_t) = 0;

        // setter
        // always return values, nonthrow, when returning
        //   true: passed state/type set successfully
        //  false: can't set passed state/type because not supported or logic constrait
        //
        //  only return true the operation makes sense, otherwise nothing changes
        virtual bool SetType (uint8_t, bool) = 0;
        virtual bool SetState(uint8_t, bool) = 0;
};

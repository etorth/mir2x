/*
 * =====================================================================================
 *
 *       Filename: activeobject.hpp
 *        Created: 04/11/2016 19:54:41
 *  Last Modified: 03/22/2017 17:21:47
 *
 *    Description: server object with
 *              
 *                  --Type()
 *                  --State()
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
// TODO: define life circle of an active object:
//
//  1. STATE_EMBRYO
//  2. STATE_INCARNATED
//  3. STATE_PHANTOM
//
//

enum ObjectType: uint8_t
{
    TYPE_NONE,

    TYPE_CREATURE,
    TYPE_MONSTER,
    TYPE_HUMAN,
    TYPE_PLAYER,
    TYPE_NPC,
    TYPE_ANIMAL,
};

enum ObjectState: uint8_t
{
    // three states of an active object
    STATE_NONE = 0,

    STATE_LIFECYCLE,
    STATE_EMBRYO,
    STATE_INCARNATED,
    STATE_PHANTOM,

    STATE_MOTION,
    STATE_MOVING,
    STATE_DEAD,

    STATE_ACTION,
    STATE_STAND,
    STATE_WALK,
    STATE_ATTACK,
    STATE_DIE,

    STATE_MODE,
    STATE_NEVERDIE,
    STATE_ATTACKALL,
    STATE_PEACE,
    STATE_CANMOVE,
    STATE_WAITMOVE,
};

class ActiveObject: public ReactObject
{
    protected:
        std::array< uint8_t, 255> m_TypeV;
        std::array< uint8_t, 255> m_StateV;
        std::array<uint32_t, 255> m_StateTimeV;

    public:
        ActiveObject()
            : ReactObject(CATEGORY_ACTIVEOBJECT)
        {
            m_TypeV.fill(0);
            m_StateV.fill(0);
            m_StateTimeV.fill(0);
        }

    public:
        // static type information
        // can safely called outside of ActiveObject
        // always return values, nonthrow, holds two types of type_info
        //      1. Type(EYE_COLOR)  : EYE_RED
        //                            EYE_BLACK
        //                            EYE_BROWN
        //
        //      2. Type(TYPE_HUMAN) : x // yes
        //                            0 // no
        // for type_info with a parameter, it use uint8_t as 255 possibilities
        // for type_info with 0 / 1 result it return 0 / x as false / true
        uint8_t Type(uint8_t nType) const
        {
            return m_TypeV[nType];
        }

    protected:
        void ResetType(uint8_t nTypeLoc, uint8_t nTypeValue)
        {
            m_TypeV[nTypeLoc] = nTypeValue;
        }

    protected:
        uint8_t  State(uint8_t);
        uint32_t StateTime(uint8_t);
        void     ResetState(uint8_t, uint8_t);
};

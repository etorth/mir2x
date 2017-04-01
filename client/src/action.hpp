/*
 * =====================================================================================
 *
 *       Filename: action.hpp
 *        Created: 03/31/2017 16:40:47
 *  Last Modified: 03/31/2017 17:02:42
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
#include "protocoldef.hpp"

class Action
{
    private:
        ActionType m_Action;

    public:
        explicit Action(ActionType nAction = ACTION_NONE)
            : m_Action(nAction)
        {}

    public:
        operator int ()
        {
            return (int)(ID());
        }

    public:
        ActionType ID() const
        {
            return m_Action;
        }

    public:
        int GfxID() const
        {
            switch(m_Action){
                case ACTION_STAND   : return  0;
                case ACTION_WALK    : return  1;
                case ACTION_ATTACK  : return  2;
                case ACTION_DIE     : return  3;
                default             : return -1;
            }
        }

    public:
        const char *Name() const
        {
            switch(m_Action){
                case ACTION_STAND   : return "ACTION_STAND" ;
                case ACTION_WALK    : return "ACTION_WALK"  ;
                case ACTION_ATTACK  : return "ACTION_ATTACK";
                case ACTION_DIE     : return "ACTION_DIE"   ;
                default             : return "ACTION_NONE"  ;
            }
        }
};

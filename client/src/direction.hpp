/*
 * =====================================================================================
 *
 *       Filename: direction.hpp
 *        Created: 03/31/2017 16:46:39
 *  Last Modified: 03/31/2017 17:00:36
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

class Direction
{
    private:
        DirectionType m_Direction;

    public:
        explicit Direction(DirectionType nDirection = DIR_NONE)
            : m_Direction(nDirection)
        {}

    public:
        operator int ()
        {
            return (int)(ID());
        }

    public:
        DirectionType ID() const
        {
            return m_Direction;
        }

    public:
        int GfxID() const
        {
            switch(m_Direction){
                case DIR_UP        : return  0;
                case DIR_DOWN      : return  4;
                case DIR_LEFT      : return  6;
                case DIR_RIGHT     : return  2;
                case DIR_UPLEFT    : return  7;
                case DIR_UPRIGHT   : return  1;
                case DIR_DOWNLEFT  : return  5;
                case DIR_DOWNRIGHT : return  3;
                default            : return -1;
            }
        }

    public:
        const char *Name() const
        {
            switch(m_ID){
                case DIR_UP        : return "DIR_UP"       ;
                case DIR_DOWN      : return "DIR_DOWN"     ;
                case DIR_LEFT      : return "DIR_LEFT"     ;
                case DIR_RIGHT     : return "DIR_RIGHT"    ;
                case DIR_UPLEFT    : return "DIR_UPLEFT"   ;
                case DIR_UPRIGHT   : return "DIR_UPRIGHT"  ;
                case DIR_DOWNLEFT  : return "DIR_DOWNLEFT" ;
                case DIR_DOWNRIGHT : return "DIR_DOWNRIGHT";
                default            : return "DIR_NONE"     ;
            }
        }
};

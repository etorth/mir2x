/*
 * =====================================================================================
 *
 *       Filename: wilanimationinfo.cpp
 *        Created: 08/08/2015 04:41:53 AM
 *  Last Modified: 04/14/2017 18:57:14
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

#include "wilanimationinfo.hpp"

int WilAnimationStartBaseIndex(int, int nAnimationIndex, int nStatus, int nDirection)
{
    if(0){
    }else{
        // default
        return nAnimationIndex * 1000 + (nStatus * 8 + nDirection) * 10;
    }
}

int WilAnimationFrameCount(int, int, int nStatus, int)
{
    if(0){
    }else{
        // default
        switch(nStatus){
            case 0:
                return 4;
            case 1:
                return 6;
            case 2:
                return 6;
            case 3:
                return 2;
            case 4:
                return 10;
            default:
                return 0;
        }
    }
}
